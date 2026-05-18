import asyncio
import threading
import tkinter as tk
from tkinter import filedialog, messagebox
from PIL import Image, ImageSequence
from bleak import BleakScanner, BleakClient

CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class LedStreamerApp(tk.Tk):
    def __init__(self):
        super().__init__()
        
        self.title("8x8 LED Matrix GIF Streamer")
        self.geometry("450x460") 
        self.configure(bg="#222222")
        
        self.client = None
        self.ble_loop = None
        self.is_streaming = False
        self.gif_frames = [] 
        self.frame_duration = 0.05 
        self.discovered_devices = [] 

        # --- UI 레이아웃 구성 ---
        self.status_label = tk.Label(self, text="상태: 장치 스캔을 시작해 주세요.", fg="white", bg="#222222", font=("맑은 고딕", 11, "bold"))
        self.status_label.pack(pady=15)

        self.device_listbox = tk.Listbox(self, width=50, height=8, bg="#333333", fg="white", selectbackground="#444444", font=("맑은 고딕", 10))
        self.device_listbox.pack(pady=10)

        btn_frame1 = tk.Frame(self, bg="#222222")
        btn_frame1.pack(pady=5)

        self.scan_btn = tk.Button(btn_frame1, text="1. 주변 장치 검색 (Scan)", width=18, command=self.start_scan_thread)
        self.scan_btn.pack(side=tk.LEFT, padx=5)

        # [변경] 연결/해제를 동적으로 처리하기 위해 command를 통합 관리 함수로 지정
        self.connect_btn = tk.Button(btn_frame1, text="2. 선택한 장치 연결", width=18, state="disabled", command=self.toggle_connection)
        self.connect_btn.pack(side=tk.LEFT, padx=5)

        self.file_btn = tk.Button(self, text="3. GIF 파일 불러오기 및 전송", width=38, height=2, state="disabled", command=self.load_gif)
        self.file_btn.pack(pady=15)

        self.test_btn = tk.Button(self, text="4. 기본 테스트 패턴 전송 (8x8 Checker)", width=38, height=2, state="disabled", fg="#222222", bg="#A3E4D7", font=("맑은 고딕", 9, "bold"), command=self.start_test_pattern)
        self.test_btn.pack(pady=5)
        
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
            self.status_label.config(text="상태: 발견된 장치가 없습니다. 다시 시도하세요.")
        else:
            self.status_label.config(text=f"상태: 스캔 완료 ({len(self.discovered_devices)}개 발견). 장치를 선택하세요.")
            for dev in self.discovered_devices:
                name = dev.name if dev.name else "Unknown Device"
                self.device_listbox.insert(tk.END, f"{name}  [{dev.address}]")
            # 연결 버튼 복구 및 텍스트 리셋
            self.connect_btn.config(state="normal", text="2. 선택한 장치 연결", fg="black", bg="#F0F0F0")
        self.scan_btn.config(state="normal")

    # --- [추가] 원버튼 토글 제어 핵심 분기 로직 ---
    def toggle_connection(self):
        # 1. 이미 연결된 상태라면 -> 해제 프로세스 진행
        if self.client and self.client.is_connected:
            self.is_streaming = False # 스트리밍 중단
            self.status_label.config(text="상태: BLE 연결 해제 중...")
            self.connect_btn.config(state="disabled")
            asyncio.run_coroutine_threadsafe(self.disconnect_ble(), self.ble_loop)
        
        # 2. 연결이 안 된 상태라면 -> 새로운 연결 프로세스 진행
        else:
            selected_index = self.device_listbox.curselection()
            if not selected_index:
                messagebox.showwarning("알림", "목록에서 연결할 블루투스 장치를 먼저 선택해 주세요!")
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
            
            # 메인 GUI 스레드에서 UI를 [연결 해제] 모드로 즉시 변경
            self.run_in_main_thread(lambda: self.status_label.config(text=f"상태: 연결 성공! ({device.name})"))
            self.run_in_main_thread(lambda: self.connect_btn.config(state="normal", text="2. 연결 해제 (Disconnect)", fg="white", bg="#EC7063"))
            self.run_in_main_thread(lambda: self.file_btn.config(state="normal"))
            self.run_in_main_thread(lambda: self.test_btn.config(state="normal")) 
        except Exception as e:
            self.run_in_main_thread(lambda: self.status_label.config(text=f"연결 실패: {str(e)}"))
            self.run_in_main_thread(lambda: self.connect_btn.config(state="normal", text="2. 선택한 장치 연결", fg="black", bg="#F0F0F0"))
        self.run_in_main_thread(lambda: self.scan_btn.config(state="normal"))

    # --- [추가] 비동기 연결 해제 함수 ---
    async def disconnect_ble(self):
        try:
            if self.client:
                await self.client.disconnect()
            self.run_in_main_thread(self.handle_disconnected_ui)
        except Exception as e:
            self.run_in_main_thread(lambda: self.status_label.config(text=f"해제 오류: {str(e)}"))
            self.run_in_main_thread(lambda: self.connect_btn.config(state="normal"))

    # --- [추가] 연결이 끊겼을 때 UI 소스 원상복구 로직 ---
    def handle_disconnected_ui(self):
        self.status_label.config(text="상태: BLE 연결이 해제되었습니다.")
        self.connect_btn.config(state="normal", text="2. 선택한 장치 연결", fg="black", bg="#F0F0F0")
        self.scan_btn.config(state="normal")
        # 데이터 전송 버튼들 다시 잠그기
        self.file_btn.config(state="disabled")
        self.test_btn.config(state="disabled")

    def load_gif(self):
        file_path = filedialog.askopenfilename(filetypes=[("GIF files", "*.gif")])
        if not file_path:
            return
        
        try:
            with Image.open(file_path) as im:
                self.gif_frames.clear()
                self.frame_duration = im.info.get('duration', 50) / 1000.0 
                
                for frame in ImageSequence.Iterator(im):
                    resized = frame.convert("L").resize((8, 8), Image.Resampling.LANCZOS)
                    frame_bytes = bytearray(resized.tobytes())
                    self.gif_frames.append(frame_bytes)
            
            self.status_label.config(text=f"상태: GIF 전송 중 ({len(self.gif_frames)} 프레임 반복)")
            self.is_streaming = True
            asyncio.run_coroutine_threadsafe(self.stream_gif(), self.ble_loop)
            
        except Exception as e:
            self.status_label.config(text=f"GIF 변환 실패: {str(e)}")

    async def stream_gif(self):
        frame_idx = 0
        while self.is_streaming and self.client and self.client.is_connected:
            if not self.gif_frames:
                await asyncio.sleep(0.1)
                continue
                
            current_frame = self.gif_frames[frame_idx]
            await self.client.write_gatt_char(CHARACTERISTIC_UUID, current_frame, response=False)
            
            frame_idx = (frame_idx + 1) % len(self.gif_frames)
            await asyncio.sleep(self.frame_duration)

    def start_test_pattern(self):
        self.status_label.config(text="상태: 8x8 체커보드 테스트 데이터 실시간 전송 중...")
        self.is_streaming = False 
        
        self.gif_frames.clear()
        self.frame_duration = 0.1 

        for frame_step in range(8):
            test_frame = bytearray(64)
            for y in range(8):
                for x in range(8):
                    if (x + y + frame_step) % 2 == 0:
                        test_frame[(y * 8) + x] = 255 
                    else:
                        test_frame[(y * 8) + x] = 0
            self.gif_frames.append(test_frame)

        self.is_streaming = True
        asyncio.run_coroutine_threadsafe(self.stream_gif(), self.ble_loop)

    def run_in_main_thread(self, func):
        self.after(0, func)

if __name__ == "__main__":
    app = LedStreamerApp()
    app.mainloop()