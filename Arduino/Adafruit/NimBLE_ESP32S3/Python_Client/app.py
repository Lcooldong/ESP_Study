import asyncio
import threading
import tkinter as tk
from tkinter import filedialog, messagebox, colorchooser
from PIL import Image, ImageSequence
from bleak import BleakScanner, BleakClient

CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class LedStreamerApp(tk.Tk):
    def __init__(self):
        super().__init__()
        
        self.title("8x8 LED Matrix Controller")
        self.geometry("480x930") # 정지 버튼 추가로 세로 길이 약간 확장
        self.configure(bg="#222222")
        
        self.client = None
        self.ble_loop = None
        self.is_streaming = False
        self.gif_frames = [] 
        self.frame_duration = 0.05 
        self.discovered_devices = [] 

        self.custom_pattern = bytearray(64)
        self.tft_color = (0, 255, 0)

        self._build_ui()
        
        self.ble_thread = threading.Thread(target=self.run_ble_loop, daemon=True)
        self.ble_thread.start()

    def _build_ui(self):
        self.status_label = tk.Label(self, text="상태: 장치 스캔을 시작해 주세요.", fg="white", bg="#222222", font=("맑은 고딕", 11, "bold"))
        self.status_label.pack(pady=10)

        self.device_listbox = tk.Listbox(self, width=55, height=6, bg="#333333", fg="white", selectbackground="#444444", font=("맑은 고딕", 10))
        self.device_listbox.pack(pady=5)

        btn_frame1 = tk.Frame(self, bg="#222222")
        btn_frame1.pack(pady=5)

        self.scan_btn = tk.Button(btn_frame1, text="1. 장치 검색", width=16, command=self.start_scan_thread)
        self.scan_btn.pack(side=tk.LEFT, padx=5)

        self.connect_btn = tk.Button(btn_frame1, text="2. 장치 연결", width=16, state="disabled", command=self.toggle_connection)
        self.connect_btn.pack(side=tk.LEFT, padx=5)

        # 8x8 인터랙티브 UI 그리드
        grid_container = tk.Frame(self, bg="#333333", bd=2, relief="sunken")
        grid_container.pack(pady=15)
        
        tk.Label(grid_container, text="[ 사용자 정의 픽셀 캔버스 ]\n칸을 클릭하여 밝기 및 색상을 변경하세요", bg="#333333", fg="#AAAAAA", font=("맑은 고딕", 9)).pack(pady=5)

        self.grid_frame = tk.Frame(grid_container, bg="#111111")
        self.grid_frame.pack(padx=10, pady=10)

        self.cells = {}
        for y in range(8):
            for x in range(8):
                canvas = tk.Canvas(self.grid_frame, width=35, height=35, bg="#222222", highlightthickness=1, highlightbackground="#444444")
                canvas.grid(row=y, column=x, padx=1, pady=1)
                canvas.bind("<Button-1>", lambda event, cx=x, cy=y: self.open_pixel_popup(cx, cy))
                self.cells[(x, y)] = canvas

        # 제어 버튼들
        self.file_btn = tk.Button(self, text="3. GIF 파일 스트리밍", width=42, height=2, state="disabled", command=self.load_gif)
        self.file_btn.pack(pady=5)

        self.test_btn = tk.Button(self, text="4. 기본 체커보드 패턴 스트리밍", width=42, height=2, state="disabled", fg="#222222", bg="#A3E4D7", font=("맑은 고딕", 9, "bold"), command=self.start_test_pattern)
        self.test_btn.pack(pady=5)

        # [신규 추가] 스트리밍 강제 정지 및 화면 초기화 버튼
        self.stop_btn = tk.Button(self, text="🛑 스트리밍 정지 (화면 초기화)", width=42, height=2, state="disabled", bg="#C0392B", fg="white", font=("맑은 고딕", 9, "bold"), command=self.stop_streaming)
        self.stop_btn.pack(pady=5)

        self.color_btn = tk.Button(self, text="🎨 전체 TFT 모니터 색상 일괄 변경", width=42, height=2, state="disabled", bg="#F39C12", fg="white", font=("맑은 고딕", 9, "bold"), command=self.change_global_tft_color)
        self.color_btn.pack(pady=5)


    # --- [신규 핵심 기능] 스트리밍 멈춤 및 화면 지우기 ---
    def stop_streaming(self):
        self.is_streaming = False # 비동기 루프 강제 탈출 플래그
        self.status_label.config(text="상태: 스트리밍 정지됨 (화면 초기화)")
        
        # 64개의 픽셀을 모두 0(꺼짐)으로 만든 데이터 생성
        empty_frame = bytearray(64)
        self.custom_pattern = empty_frame
        
        # UI 캔버스 까맣게 업데이트
        self.refresh_all_canvases()
        
        # 아두이노 보드로 꺼짐 데이터 즉시 전송
        if self.client and self.client.is_connected:
            asyncio.run_coroutine_threadsafe(self.client.write_gatt_char(CHARACTERISTIC_UUID, empty_frame, response=False), self.ble_loop)


    def open_pixel_popup(self, x, y):
        popup = tk.Toplevel(self)
        popup.geometry("280x250")
        popup.title(f"픽셀 ({x}, {y}) 제어 패널")
        popup.configure(bg="#2C3E50")

        tk.Label(popup, text=f"현재 선택된 픽셀: X={x}, Y={y}", bg="#2C3E50", fg="white", font=("맑은 고딕", 10, "bold")).pack(pady=10)

        tk.Label(popup, text="밝기 (Brightness):", bg="#2C3E50", fg="#BDC3C7").pack()
        brightness_var = tk.IntVar(value=self.custom_pattern[y * 8 + x])
        scale = tk.Scale(popup, variable=brightness_var, from_=0, to=255, orient="horizontal", bg="#34495E", fg="white", highlightthickness=0, length=200)
        scale.pack(pady=5)

        def apply_brightness():
            val = brightness_var.get()
            self.custom_pattern[y * 8 + x] = val
            self.update_single_canvas(x, y)
            
            self.is_streaming = False 
            self.send_custom_frame()
            popup.destroy()

        tk.Button(popup, text="✔ 밝기 적용 및 전송", bg="#27AE60", fg="white", width=20, command=apply_brightness).pack(pady=10)

        def pick_color():
            color_code = colorchooser.askcolor(title="디스플레이 색상 선택")
            if color_code[0]:
                r, g, b = [int(c) for c in color_code[0]]
                self.tft_color = (r, g, b)
                self.send_tft_color(r, g, b)
                self.refresh_all_canvases()
                popup.destroy()

        tk.Button(popup, text="🎨 색상 변경 (전체 화면 적용)", bg="#E67E22", fg="white", width=20, command=pick_color).pack(pady=5)

    def update_single_canvas(self, x, y):
        brightness = self.custom_pattern[y * 8 + x]
        if brightness == 0:
            hex_color = "#222222" 
        else:
            r = int((self.tft_color[0] * brightness) / 255)
            g = int((self.tft_color[1] * brightness) / 255)
            b = int((self.tft_color[2] * brightness) / 255)
            hex_color = f"#{r:02x}{g:02x}{b:02x}"
        self.cells[(x, y)].config(bg=hex_color)

    def refresh_all_canvases(self):
        for y in range(8):
            for x in range(8):
                self.update_single_canvas(x, y)

    def send_custom_frame(self):
        if self.client and self.client.is_connected:
            asyncio.run_coroutine_threadsafe(self.client.write_gatt_char(CHARACTERISTIC_UUID, self.custom_pattern, response=False), self.ble_loop)

    def send_tft_color(self, r, g, b):
        if self.client and self.client.is_connected:
            color_bytes = bytearray([r, g, b])
            asyncio.run_coroutine_threadsafe(self.client.write_gatt_char(CHARACTERISTIC_UUID, color_bytes, response=False), self.ble_loop)
            self.status_label.config(text=f"상태: 색상 변경 적용 완료 (R:{r} G:{g} B:{b})")

    def change_global_tft_color(self):
        color_code = colorchooser.askcolor(title="TFT 디스플레이 격자 색상 선택")
        if color_code[0]:
            r, g, b = [int(x) for x in color_code[0]]
            self.tft_color = (r, g, b)
            self.send_tft_color(r, g, b)
            self.refresh_all_canvases()

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
            self.connect_btn.config(state="normal", text="2. 장치 연결", fg="black", bg="#F0F0F0")
        self.scan_btn.config(state="normal")

    def toggle_connection(self):
        if self.client and self.client.is_connected:
            self.is_streaming = False 
            self.status_label.config(text="상태: BLE 연결 해제 중...")
            self.connect_btn.config(state="disabled")
            asyncio.run_coroutine_threadsafe(self.disconnect_ble(), self.ble_loop)
        else:
            selected_index = self.device_listbox.curselection()
            if not selected_index:
                messagebox.showwarning("알림", "연결할 블루투스 장치를 선택해 주세요!")
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
            self.run_in_main_thread(lambda: self.status_label.config(text=f"상태: 연결 성공! ({device.name})"))
            self.run_in_main_thread(lambda: self.connect_btn.config(state="normal", text="2. 연결 해제", fg="white", bg="#EC7063"))
            self.run_in_main_thread(lambda: self.file_btn.config(state="normal"))
            self.run_in_main_thread(lambda: self.test_btn.config(state="normal")) 
            self.run_in_main_thread(lambda: self.stop_btn.config(state="normal")) # [정지 버튼 활성화]
            self.run_in_main_thread(lambda: self.color_btn.config(state="normal"))
        except Exception as e:
            self.run_in_main_thread(lambda: self.status_label.config(text=f"연결 실패: {str(e)}"))
            self.run_in_main_thread(lambda: self.connect_btn.config(state="normal", text="2. 장치 연결", fg="black", bg="#F0F0F0"))
        self.run_in_main_thread(lambda: self.scan_btn.config(state="normal"))

    async def disconnect_ble(self):
        try:
            if self.client:
                await self.client.disconnect()
            self.run_in_main_thread(self.handle_disconnected_ui)
        except Exception as e:
            self.run_in_main_thread(lambda: self.status_label.config(text=f"해제 오류: {str(e)}"))
            self.run_in_main_thread(lambda: self.connect_btn.config(state="normal"))

    def handle_disconnected_ui(self):
        self.status_label.config(text="상태: BLE 연결이 해제되었습니다.")
        self.connect_btn.config(state="normal", text="2. 장치 연결", fg="black", bg="#F0F0F0")
        self.scan_btn.config(state="normal")
        self.file_btn.config(state="disabled")
        self.test_btn.config(state="disabled")
        self.stop_btn.config(state="disabled") # [정지 버튼 비활성화]
        self.color_btn.config(state="disabled")

    def load_gif(self):
        file_path = filedialog.askopenfilename(filetypes=[("GIF files", "*.gif")])
        if not file_path: return
        try:
            self.is_streaming = False # [안전장치] 기존 스트리밍이 있다면 탈출시킴
            with Image.open(file_path) as im:
                self.gif_frames.clear()
                self.frame_duration = im.info.get('duration', 50) / 1000.0 
                for frame in ImageSequence.Iterator(im):
                    resized = frame.convert("L").resize((8, 8), Image.Resampling.LANCZOS)
                    self.gif_frames.append(bytearray(resized.tobytes()))
            self.status_label.config(text=f"상태: GIF 전송 중 ({len(self.gif_frames)} 프레임 반복)")
            self.is_streaming = True
            asyncio.run_coroutine_threadsafe(self.stream_gif(), self.ble_loop)
        except Exception as e:
            self.status_label.config(text=f"GIF 변환 실패: {str(e)}")

    def start_test_pattern(self):
        self.is_streaming = False # [안전장치] 기존 스트리밍 탈출
        self.status_label.config(text="상태: 체커보드 실시간 전송 중...")
        
        self.gif_frames.clear()
        self.frame_duration = 0.1 

        for frame_step in range(8):
            test_frame = bytearray(64)
            for y in range(8):
                for x in range(8):
                    if (x + y + frame_step) % 2 == 0:
                        test_frame[(y * 8) + x] = 255 
            self.gif_frames.append(test_frame)

        self.is_streaming = True
        asyncio.run_coroutine_threadsafe(self.stream_gif(), self.ble_loop)

    async def stream_gif(self):
        frame_idx = 0
        while self.is_streaming and self.client and self.client.is_connected:
            if not self.gif_frames:
                await asyncio.sleep(0.1)
                continue
            current_frame = self.gif_frames[frame_idx]
            
            try:
                await self.client.write_gatt_char(CHARACTERISTIC_UUID, current_frame, response=False)
            except Exception as e:
                print(f"전송 에러 (스트리밍 중단): {e}")
                self.is_streaming = False
                break
                
            self.run_in_main_thread(lambda f=current_frame: self.sync_ui_with_frame(f))
            
            frame_idx = (frame_idx + 1) % len(self.gif_frames)
            await asyncio.sleep(self.frame_duration)

    def sync_ui_with_frame(self, frame_bytes):
        if self.is_streaming: # 스트리밍이 멈췄을 땐 UI를 업데이트하지 않음
            self.custom_pattern = bytearray(frame_bytes)
            self.refresh_all_canvases()

    def run_in_main_thread(self, func):
        self.after(0, func)

if __name__ == "__main__":
    app = LedStreamerApp()
    app.mainloop()