import asyncio
import threading
import tkinter as tk
from tkinter import filedialog, messagebox
from bleak import BleakScanner, BleakClient

CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class BleTestClientApp(tk.Tk):
    def __init__(self):
        super().__init__()
        
        self.title("ESP32-S3 BLE LED Controller")
        self.geometry("450x460") # 상태 표시 라벨 추가로 세로 크기 약간 확장
        self.configure(bg="#222222")
        
        self.client = None
        self.ble_loop = None
        self.discovered_devices = [] 

        # --- UI 레이아웃 구성 ---
        self.status_label = tk.Label(self, text="상태: 장치 검색을 시작해 주세요.", fg="white", bg="#222222", font=("맑은 고딕", 11, "bold"))
        self.status_label.pack(pady=15)

        # [추가] 아두이노로부터 피드백받은 LED 상태를 표시할 UI 라벨
        self.hw_status_label = tk.Label(self, text="하드웨어 LED 상태: 알 수 없음", fg="#AAAAAA", bg="#111111", font=("맑은 고딕", 11, "bold"), width=35, height=2)
        self.hw_status_label.pack(pady=5)

        self.device_listbox = tk.Listbox(self, width=50, height=8, bg="#333333", fg="white", selectbackground="#444444", font=("맑은 고딕", 10))
        self.device_listbox.pack(pady=10)

        btn_frame1 = tk.Frame(self, bg="#222222")
        btn_frame1.pack(pady=5)

        self.scan_btn = tk.Button(btn_frame1, text="1. 장치 검색 (Scan)", width=18, command=self.start_scan_thread)
        self.scan_btn.pack(side=tk.LEFT, padx=5)

        self.connect_btn = tk.Button(btn_frame1, text="2. 선택한 장치 연결", width=18, state="disabled", command=self.toggle_connection)
        self.connect_btn.pack(side=tk.LEFT, padx=5)

        # LED 제어부 프레임 (양옆 배치)
        btn_frame2 = tk.Frame(self, bg="#222222")
        btn_frame2.pack(pady=20)

        self.led_on_btn = tk.Button(btn_frame2, text="LED 켜기 (ON)", width=18, height=2, state="disabled", fg="white", bg="#27AE60", font=("맑은 고딕", 10, "bold"), command=lambda: self.send_command("1"))
        self.led_on_btn.pack(side=tk.LEFT, padx=10)

        self.led_off_btn = tk.Button(btn_frame2, text="LED 끄기 (OFF)", width=18, height=2, state="disabled", fg="white", bg="#C0392B", font=("맑은 고딕", 10, "bold"), command=lambda: self.send_command("0"))
        self.led_off_btn.pack(side=tk.LEFT, padx=10)
        
        self.ble_thread = threading.Thread(target=self.run_ble_loop, daemon=True)
        self.ble_thread.start()

    def run_ble_loop(self):
        self.ble_loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.ble_loop)
        self.ble_loop.run_forever()

    def start_scan_thread(self):
        self.status_label.config(text="상태: 주변 블루투스 기기 검색 중 (5초)...")
        self.scan_btn.config(state="disabled")
        self.connect_btn.config(state="disabled")
        self.device_listbox.delete(0, tk.END)
        asyncio.run_coroutine_threadsafe(self.scan_ble(), self.ble_loop)

    async def scan_ble(self):
        try:
            devices = await BleakScanner.discover(timeout=5.0)
            self.discovered_devices = list(devices)
            self.run_in_main_thread(self.update_device_list)
        except Exception as e:
            self.run_in_main_thread(lambda: self.status_label.config(text=f"스캔 오류: {str(e)}"))
            self.run_in_main_thread(lambda: self.scan_btn.config(state="normal"))

    def update_device_list(self):
        if not self.discovered_devices:
            self.status_label.config(text="상태: 발견된 장치가 없습니다.")
        else:
            self.status_label.config(text=f"상태: 스캔 완료 ({len(self.discovered_devices)}개 발견). 장치를 선택하세요.")
            for dev in self.discovered_devices:
                name = dev.name if dev.name else "Unknown Device"
                self.device_listbox.insert(tk.END, f"{name}  [{dev.address}]")
            self.connect_btn.config(state="normal", text="2. 선택한 장치 연결", fg="black", bg="#F0F0F0")
        self.scan_btn.config(state="normal")

    def toggle_connection(self):
        if self.client and self.client.is_connected:
            self.status_label.config(text="상태: BLE 연결 해제 중...")
            self.connect_btn.config(state="disabled")
            asyncio.run_coroutine_threadsafe(self.disconnect_ble(), self.ble_loop)
        else:
            selected_index = self.device_listbox.curselection()
            if not selected_index:
                messagebox.showwarning("알림", "목록에서 연결할 블루투스 장치를 선택해 주세요!")
                return
                
            target_device = self.discovered_devices[selected_index[0]]
            self.status_label.config(text=f"상태: {target_device.address} 연결 시도 중...")
            self.connect_btn.config(state="disabled")
            self.scan_btn.config(state="disabled")
            asyncio.run_coroutine_threadsafe(self.connect_ble(target_device), self.ble_loop)

    async def connect_ble(self, device):
        try:
            self.client = BleakClient(device.address)
            await self.client.connect()
            
            # [추가] 연결 성공 직후, 아두이노의 Notify(실시간 알림) 수신을 활성화
            await self.client.start_notify(CHARACTERISTIC_UUID, self.notification_handler)
            
            self.run_in_main_thread(lambda: self.status_label.config(text=f"상태: 연결 성공! ({device.name})"))
            self.run_in_main_thread(lambda: self.connect_btn.config(state="normal", text="2. 연결 해제 (Disconnect)", fg="white", bg="#EC7063"))
            self.run_in_main_thread(lambda: self.led_on_btn.config(state="normal"))
            self.run_in_main_thread(lambda: self.led_off_btn.config(state="normal"))
            
            # 처음 연결 시 보드에 설정된 초기 LED 상태 읽어오기 (수동 Read)
            init_state = await self.client.read_gatt_char(CHARACTERISTIC_UUID)
            self.run_in_main_thread(lambda: self.update_hw_status_ui(init_state.decode('utf-8')))
            
        except Exception as e:
            self.run_in_main_thread(lambda: self.status_label.config(text=f"연결 실패: {str(e)}"))
            self.run_in_main_thread(lambda: self.connect_btn.config(state="normal", text="2. 선택한 장치 연결", fg="black", bg="#F0F0F0"))
        self.run_in_main_thread(lambda: self.scan_btn.config(state="normal"))

    # [추가] 아두이노가 pChar->notify()를 날릴 때마다 비동기로 자동 호출되는 콜백 함수
    def notification_handler(self, sender, data):
        state_str = data.decode('utf-8')
        # 수신 데이터를 메인 GUI 스레드로 안전하게 전달
        self.run_in_main_thread(lambda: self.update_hw_status_ui(state_str))

    # [추가] 수신한 상태 값에 따라 상단 라벨 창 색상과 글자를 바꾸는 GUI 함수
    def update_hw_status_ui(self, state_str):
        if state_str == "1":
            self.hw_status_label.config(text="하드웨어 LED 상태: [ ON ]", fg="#2ECC71") # 초록색 글씨
        elif state_str == "0":
            self.hw_status_label.config(text="하드웨어 LED 상태: [ OFF ]", fg="#E74C3C") # 빨간색 글씨

    async def disconnect_ble(self):
        try:
            if self.client:
                # 연결을 완전히 끊기 전 Notify 수신 해제 (안전장치)
                if self.client.is_connected:
                    await self.client.stop_notify(CHARACTERISTIC_UUID)
                await self.client.disconnect()
            self.run_in_main_thread(self.handle_disconnected_ui)
        except Exception as e:
            self.run_in_main_thread(lambda: self.status_label.config(text=f"해제 오류: {str(e)}"))
            self.run_in_main_thread(lambda: self.connect_btn.config(state="normal"))

    def handle_disconnected_ui(self):
        self.status_label.config(text="상태: BLE 연결이 해제되었습니다.")
        self.hw_status_label.config(text="하드웨어 LED 상태: 연결 없음", fg="#AAAAAA")
        self.connect_btn.config(state="normal", text="2. 선택한 장치 연결", fg="black", bg="#F0F0F0")
        self.scan_btn.config(state="normal")
        self.led_on_btn.config(state="disabled")
        self.led_off_btn.config(state="disabled")

    def send_command(self, cmd_string):
        if self.client and self.client.is_connected:
            asyncio.run_coroutine_threadsafe(self._write_ble(cmd_string), self.ble_loop)

    async def _write_ble(self, cmd_string):
        try:
            await self.client.write_gatt_char(CHARACTERISTIC_UUID, cmd_string.encode('utf-8'), response=True)
        except Exception as e:
            print(f"데이터 전송 에러: {e}")

    def run_in_main_thread(self, func):
        self.after(0, func)

if __name__ == "__main__":
    app = BleTestClientApp()
    app.mainloop()