# pip install bleak Pillow numpy
import sys
import subprocess
import importlib


def install_if_missing(package_name, import_name=None):
    if import_name is None:
        import_name = package_name

    try:
        importlib.import_module(import_name)
    except ImportError:
        print(f"{package_name} 모듈이 없어 설치합니다...")
        subprocess.check_call([
            sys.executable,
            "-m",
            "pip",
            "install",
            package_name
        ])


install_if_missing("bleak")
install_if_missing("Pillow", "PIL")
install_if_missing("numpy")

import asyncio
import threading
import tkinter as tk
from tkinter import filedialog, messagebox
from bleak import BleakScanner, BleakClient
from PIL import Image, ImageSequence, ImageDraw, ImageFont
import numpy as np

CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class LedStreamerApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("8x8 LED Matrix RGB Controller")
        self.geometry("500x950")
        self.configure(bg="#222222")
        
        self.client = None
        self.ble_loop = None
        self.is_streaming = False
        self.gif_frames = []
        self.stream_type = "gif"
        self.active_popup = None
        
        self.pixels = [{'r': 255, 'g': 255, 'b': 255, 'br': 0} for _ in range(64)]
        self._debounce_timer = None
        
        self._build_ui()
        self.ble_thread = threading.Thread(target=self.run_ble_loop, daemon=True)
        self.ble_thread.start()

    def _build_ui(self):
        self.status_label = tk.Label(self, text="상태: 장치 스캔을 시작해 주세요.", fg="white", bg="#222222", font=("맑은 고딕", 11, "bold"))
        self.status_label.pack(pady=10)

        self.file_info_label = tk.Label(self, text="현재 파일: 없음", fg="#F1C40F", bg="#222222", font=("맑은 고딕", 9))
        self.file_info_label.pack(pady=2)

        self.device_listbox = tk.Listbox(self, width=55, height=6, bg="#333333", fg="white", selectbackground="#444444", font=("맑은 고딕", 10))
        self.device_listbox.pack(pady=5)

        btn_frame1 = tk.Frame(self, bg="#222222")
        btn_frame1.pack(pady=5)

        self.scan_btn = tk.Button(btn_frame1, text="1. 장치 검색", width=16, command=self.start_scan_thread)
        self.scan_btn.pack(side=tk.LEFT, padx=5)

        self.connect_btn = tk.Button(btn_frame1, text="2. 장치 연결", width=16, state="disabled", command=self.toggle_connection)
        self.connect_btn.pack(side=tk.LEFT, padx=5)

        grid_container = tk.Frame(self, bg="#333333", bd=2, relief="sunken")
        grid_container.pack(pady=10)
        
        tk.Label(grid_container, text="[ 풀컬러 픽셀 캔버스 ] 칸 또는 R/C 버튼을 눌러 색상을 조절하세요", bg="#333333", fg="#AAAAAA", font=("맑은 고딕", 9)).pack(pady=5)

        self.grid_frame = tk.Frame(grid_container, bg="#111111")
        self.grid_frame.pack(padx=10, pady=10)

        self.reset_btn = tk.Button(self.grid_frame, text="초기화", bg="#922B21", fg="white", font=("맑은 고딕", 7, "bold"), command=self.reset_all_pixels)
        self.reset_btn.grid(row=0, column=0, padx=2, pady=2, sticky="nsew")

        for x in range(8):
            btn = tk.Button(self.grid_frame, text=f"C{x}", bg="#555555", fg="white", font=("맑은 고딕", 8, "bold"), command=lambda cx=x: self.open_line_popup('col', cx))
            btn.grid(row=0, column=x+1, padx=1, pady=2, sticky="nsew")

        for y in range(8):
            btn = tk.Button(self.grid_frame, text=f"R{y}", bg="#555555", fg="white", font=("맑은 고딕", 8, "bold"), command=lambda cy=y: self.open_line_popup('row', cy))
            btn.grid(row=y+1, column=0, padx=2, pady=1, sticky="nsew")

        self.cells = {}
        for y in range(8):
            for x in range(8):
                canvas = tk.Canvas(self.grid_frame, width=35, height=35, bg="#222222", highlightthickness=1, highlightbackground="#444444")
                canvas.grid(row=y+1, column=x+1, padx=1, pady=1)
                canvas.bind("<Button-1>", lambda event, cx=x, cy=y: self.open_pixel_popup(cx, cy))
                self.cells[(x, y)] = canvas

        self.file_btn = tk.Button(self, text="3. 단일 이미지 및 GIF 파일 불러오기", width=42, height=2, state="disabled", command=self.load_media)
        self.file_btn.pack(pady=3)

        # [신규] 텍스트 스크롤 영역
        text_frame = tk.Frame(self, bg="#222222")
        text_frame.pack(pady=3)
        self.text_entry = tk.Entry(text_frame, width=28, font=("맑은 고딕", 10))
        self.text_entry.insert(0, "HELLO")
        self.text_entry.pack(side=tk.LEFT, padx=5)
        self.text_send_btn = tk.Button(text_frame, text="전광판 전송", state="disabled", bg="#3498DB", fg="white", font=("맑은 고딕", 9, "bold"), command=self.start_text_scroll)
        self.text_send_btn.pack(side=tk.LEFT)

        self.test_btn = tk.Button(self, text="4. 기본 체커보드 패턴 스트리밍", width=42, height=2, state="disabled", fg="#222222", bg="#A3E4D7", font=("맑은 고딕", 9, "bold"), command=self.start_test_pattern)
        self.test_btn.pack(pady=3)

        self.move_test_btn = tk.Button(self, text="5. 순차적 픽셀 이동 테스트", width=42, height=2, state="disabled", fg="#222222", bg="#F9E79F", font=("맑은 고딕", 9, "bold"), command=self.start_moving_pixel_test)
        self.move_test_btn.pack(pady=3)

        # [신규] 물방울 번짐 효과 버튼
        self.ripple_btn = tk.Button(self, text="6. 물결(Ripple) 퍼짐 애니메이션", width=42, height=2, state="disabled", fg="white", bg="#8E44AD", font=("맑은 고딕", 9, "bold"), command=self.start_ripple_effect)
        self.ripple_btn.pack(pady=3)

        self.stop_btn = tk.Button(self, text="🛑 화면 끄기 (설정된 색상 유지)", width=42, height=2, state="disabled", bg="#C0392B", fg="white", font=("맑은 고딕", 9, "bold"), command=self.clear_screen)
        self.stop_btn.pack(pady=3)

        self.refresh_all_canvases()

    def reset_all_pixels(self):
        self.is_streaming = False 
        for p in self.pixels:
            p['r'], p['g'], p['b'] = 255, 255, 255
            p['br'] = 0
        self.refresh_all_canvases()
        self.trigger_ble_send()
        self.status_label.config(text="상태: 모든 격자가 초기 상태(회색 격자)로 초기화되었습니다.")

    def clear_screen(self):
        self.is_streaming = False 
        self.status_label.config(text="상태: 화면 끄기 완료 (기존 색상 보존 대기)")
        
        for p in self.pixels:
            p['br'] = 0
            
        self.refresh_all_canvases() 
        self.trigger_ble_send()

    def trigger_ble_send(self):
        self.is_streaming = False 
        if self._debounce_timer:
            self.after_cancel(self._debounce_timer)
        self._debounce_timer = self.after(30, self.send_custom_frame)

    def _close_existing_popup(self):
        if self.active_popup and tk.Toplevel.winfo_exists(self.active_popup):
            self.active_popup.destroy()

    def open_pixel_popup(self, x, y):
        self._close_existing_popup() 
        popup = tk.Toplevel(self)
        self.active_popup = popup 
        popup.geometry("280x240")
        popup.title(f"픽셀 제어")
        popup.configure(bg="#2C3E50")

        tk.Label(popup, text=f"[{x}, {y}] 픽셀 색상 및 밝기 조절", bg="#2C3E50", fg="white", font=("맑은 고딕", 10, "bold")).pack(pady=(10, 5))

        idx = y * 8 + x
        p = self.pixels[idx]
        
        current_color = 'R'
        if p['g'] == 255 and p['r'] == 0: current_color = 'G'
        elif p['b'] == 255 and p['r'] == 0: current_color = 'B'

        color_frame = tk.Frame(popup, bg="#2C3E50")
        color_frame.pack(pady=5)

        btn_r = tk.Button(color_frame, text="RED", width=7, font=("맑은 고딕", 9, "bold"))
        btn_g = tk.Button(color_frame, text="GREEN", width=7, font=("맑은 고딕", 9, "bold"))
        btn_b = tk.Button(color_frame, text="BLUE", width=7, font=("맑은 고딕", 9, "bold"))
        
        btn_r.grid(row=0, column=0, padx=3)
        btn_g.grid(row=0, column=1, padx=3)
        btn_b.grid(row=0, column=2, padx=3)

        def set_color(c):
            btn_r.config(bg="#E74C3C" if c == 'R' else "#555555", fg="white" if c == 'R' else "#888888")
            btn_g.config(bg="#2ECC71" if c == 'G' else "#555555", fg="black" if c == 'G' else "#888888")
            btn_b.config(bg="#3498DB" if c == 'B' else "#555555", fg="white" if c == 'B' else "#888888")
            
            if c == 'R': p['r'], p['g'], p['b'] = 255, 0, 0
            elif c == 'G': p['r'], p['g'], p['b'] = 0, 255, 0
            elif c == 'B': p['r'], p['g'], p['b'] = 0, 0, 255
            
            self.update_single_canvas(x, y)
            self.trigger_ble_send()

        btn_r.config(command=lambda: set_color('R'))
        btn_g.config(command=lambda: set_color('G'))
        btn_b.config(command=lambda: set_color('B'))

        set_color(current_color) 

        tk.Label(popup, text="밝기 조절", bg="#2C3E50", fg="#BDC3C7").pack(pady=(10, 0))
        br_var = tk.IntVar(value=p['br'])
        
        def on_br_slide(_=None):
            p['br'] = br_var.get()
            self.update_single_canvas(x, y)
            self.trigger_ble_send()

        tk.Scale(popup, variable=br_var, from_=0, to=255, orient="horizontal", fg="white", bg="#2C3E50", troughcolor="#95A5A6", showvalue=False, highlightthickness=0, length=200, command=on_br_slide).pack()

    def open_line_popup(self, mode, index):
        self._close_existing_popup() 
        popup = tk.Toplevel(self)
        self.active_popup = popup 
        popup.geometry("280x240")
        title_text = f"열(C{index})" if mode == 'col' else f"행(R{index})"
        popup.title(f"{title_text} 일괄 제어")
        popup.configure(bg="#34495E")

        tk.Label(popup, text=f"{title_text} 전체 색상/밝기 일괄 조절", bg="#34495E", fg="#F1C40F", font=("맑은 고딕", 10, "bold")).pack(pady=(10, 5))

        init_idx = 0 * 8 + index if mode == 'col' else index * 8 + 0
        p = self.pixels[init_idx]
        
        current_color = 'R'
        if p['g'] == 255 and p['r'] == 0: current_color = 'G'
        elif p['b'] == 255 and p['r'] == 0: current_color = 'B'

        color_frame = tk.Frame(popup, bg="#34495E")
        color_frame.pack(pady=5)

        btn_r = tk.Button(color_frame, text="RED", width=7, font=("맑은 고딕", 9, "bold"))
        btn_g = tk.Button(color_frame, text="GREEN", width=7, font=("맑은 고딕", 9, "bold"))
        btn_b = tk.Button(color_frame, text="BLUE", width=7, font=("맑은 고딕", 9, "bold"))
        
        btn_r.grid(row=0, column=0, padx=3)
        btn_g.grid(row=0, column=1, padx=3)
        btn_b.grid(row=0, column=2, padx=3)

        def set_line_color(c):
            btn_r.config(bg="#E74C3C" if c == 'R' else "#555555", fg="white" if c == 'R' else "#888888")
            btn_g.config(bg="#2ECC71" if c == 'G' else "#555555", fg="black" if c == 'G' else "#888888")
            btn_b.config(bg="#3498DB" if c == 'B' else "#555555", fg="white" if c == 'B' else "#888888")
            
            for i in range(8):
                idx = i * 8 + index if mode == 'col' else index * 8 + i
                if c == 'R': self.pixels[idx]['r'], self.pixels[idx]['g'], self.pixels[idx]['b'] = 255, 0, 0
                elif c == 'G': self.pixels[idx]['r'], self.pixels[idx]['g'], self.pixels[idx]['b'] = 0, 255, 0
                elif c == 'B': self.pixels[idx]['r'], self.pixels[idx]['g'], self.pixels[idx]['b'] = 0, 0, 255
                
                if mode == 'col': self.update_single_canvas(index, i)
                else:             self.update_single_canvas(i, index)
            self.trigger_ble_send()

        btn_r.config(command=lambda: set_line_color('R'))
        btn_g.config(command=lambda: set_line_color('G'))
        btn_b.config(command=lambda: set_line_color('B'))

        set_line_color(current_color)

        tk.Label(popup, text="일괄 밝기 조절", bg="#34495E", fg="#BDC3C7").pack(pady=(10, 0))
        br_var = tk.IntVar(value=p['br'])
        
        def on_br_slide(_=None):
            val = br_var.get()
            for i in range(8):
                idx = i * 8 + index if mode == 'col' else index * 8 + i
                self.pixels[idx]['br'] = val
                if mode == 'col': self.update_single_canvas(index, i)
                else:             self.update_single_canvas(i, index)
            self.trigger_ble_send()

        tk.Scale(popup, variable=br_var, from_=0, to=255, orient="horizontal", fg="white", bg="#34495E", troughcolor="#95A5A6", showvalue=False, highlightthickness=0, length=200, command=on_br_slide).pack()

    def update_single_canvas(self, x, y):
        p = self.pixels[y * 8 + x]
        if p['br'] == 0:
            disp_r = int(p['r'] * 0.15)
            disp_g = int(p['g'] * 0.15)
            disp_b = int(p['b'] * 0.15)
        else:
            ratio = p['br'] / 255.0
            disp_r = int(p['r'] * ratio)
            disp_g = int(p['g'] * ratio)
            disp_b = int(p['b'] * ratio)
            
        hex_color = f"#{disp_r:02x}{disp_g:02x}{disp_b:02x}"
        self.cells[(x, y)].config(bg=hex_color)

    def refresh_all_canvases(self):
        for y in range(8):
            for x in range(8):
                self.update_single_canvas(x, y)

    def send_custom_frame(self):
        if self.client and self.client.is_connected:
            payload = bytearray(192)
            for i in range(64):
                p = self.pixels[i]
                ratio = p['br'] / 255.0
                payload[i*3]     = int(p['r'] * ratio)
                payload[i*3 + 1] = int(p['g'] * ratio)
                payload[i*3 + 2] = int(p['b'] * ratio)
            
            asyncio.run_coroutine_threadsafe(self.client.write_gatt_char(CHARACTERISTIC_UUID, payload, response=False), self.ble_loop)

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
                messagebox.showwarning("알림", "장치를 선택해 주세요!")
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
            self.run_in_main_thread(lambda: self.text_send_btn.config(state="normal"))
            self.run_in_main_thread(lambda: self.test_btn.config(state="normal")) 
            self.run_in_main_thread(lambda: self.move_test_btn.config(state="normal")) 
            self.run_in_main_thread(lambda: self.ripple_btn.config(state="normal")) 
            self.run_in_main_thread(lambda: self.stop_btn.config(state="normal")) 
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
        self.text_send_btn.config(state="disabled")
        self.test_btn.config(state="disabled")
        self.move_test_btn.config(state="disabled")
        self.ripple_btn.config(state="disabled")
        self.stop_btn.config(state="disabled") 

    def load_media(self):
        path = filedialog.askopenfilename(filetypes=[("Image/GIF", "*.gif *.png *.jpg *.jpeg")])
        if not path: return
        
        file_name = path.split('/')[-1]
        self.file_info_label.config(text=f"현재 파일: {file_name}")
        self.is_streaming = False 
        self.gif_frames = []
        
        ext = path.split('.')[-1].lower()
        if ext == 'gif':
            self.stream_type = "gif" 
            with Image.open(path) as im:
                self.frame_duration = im.info.get('duration', 50) / 1000.0 
                for frame in ImageSequence.Iterator(im):
                    resized = frame.convert("RGB").resize((8, 8), Image.Resampling.LANCZOS)
                    rgb_data = list(resized.getdata())
                    frame_bytes = bytearray(192)
                    for i, (r, g, b) in enumerate(rgb_data):
                        frame_bytes[i*3], frame_bytes[i*3+1], frame_bytes[i*3+2] = r, g, b
                    self.gif_frames.append(frame_bytes)
            self.is_streaming = True
            asyncio.run_coroutine_threadsafe(self.stream_gif(), self.ble_loop)
            
        else: # 일반 이미지
            img = Image.open(path).convert("RGBA")
            arr = np.array(img.resize((8, 8), Image.Resampling.LANCZOS))
            frame_bytes = bytearray(192)
            for y in range(8):
                for x in range(8):
                    r, g, b, a = arr[y, x]
                    i = y * 8 + x
                    if a < 128 or (r > 240 and g > 240 and b > 240):
                        frame_bytes[i*3] = frame_bytes[i*3+1] = frame_bytes[i*3+2] = 0
                    else:
                        frame_bytes[i*3], frame_bytes[i*3+1], frame_bytes[i*3+2] = int(r), int(g), int(b)
                        
            self.is_streaming = False 
            for i in range(64):
                self.pixels[i]['r'] = frame_bytes[i*3]
                self.pixels[i]['g'] = frame_bytes[i*3+1]
                self.pixels[i]['b'] = frame_bytes[i*3+2]
                self.pixels[i]['br'] = 255 if (frame_bytes[i*3]>0 or frame_bytes[i*3+1]>0 or frame_bytes[i*3+2]>0) else 0
            
            self.refresh_all_canvases() 
            self.send_custom_frame() 

    # --- [신규] 텍스트 스크롤러 기능 ---
    def start_text_scroll(self):
        if self.is_streaming and self.stream_type == "scroll":
            self.clear_screen()
            return
            
        text = self.text_entry.get()
        if not text: return
        
        self.is_streaming = False
        self.stream_type = "scroll"
        self.status_label.config(text=f"상태: 텍스트 스크롤 중 ('{text}')")
        
        # 기본 폰트로 텍스트를 그린 후 프레임별로 8x8 크기만큼 잘라냅니다.
        font = ImageFont.load_default()
        try:
            w = font.getbbox(text)[2]
        except AttributeError:
            w = font.getsize(text)[0]
            
        img_w = w + 16 # 화면 밖에서 들어오고 나가도록 여백 추가
        img = Image.new("L", (img_w, 8), "black")
        draw = ImageDraw.Draw(img)
        draw.text((8, -2), text, fill="white", font=font)
        
        self.gif_frames.clear()
        self.frame_duration = 0.1
        
        for offset in range(img_w - 7):
            cropped = img.crop((offset, 0, offset+8, 8))
            
            # numpy를 사용해 이미지를 바로 배열로 변환하고 1차원(flatten)으로 폅니다.
            arr = np.array(cropped).flatten()
            
            # 배열을 그대로 bytearray로 감싸주면 끝! (경고 해결 및 속도 향상)
            mask_frame = bytearray(arr)
            
            self.gif_frames.append(mask_frame)
            
        self.is_streaming = True
        asyncio.run_coroutine_threadsafe(self.stream_gif(), self.ble_loop)

    def start_test_pattern(self):
        if self.is_streaming and self.stream_type == "checker":
            self.clear_screen()
            return
        self.is_streaming = False 
        self.stream_type = "checker" 
        self.status_label.config(text="상태: 커스텀 컬러 체커보드 테스트 중...")
        self.gif_frames.clear()
        self.frame_duration = 0.1 

        for frame_step in range(8):
            mask_frame = bytearray(64)
            for y in range(8):
                for x in range(8):
                    if (x + y + frame_step) % 2 == 0: mask_frame[y * 8 + x] = 255 
            self.gif_frames.append(mask_frame)

        self.is_streaming = True
        asyncio.run_coroutine_threadsafe(self.stream_gif(), self.ble_loop)

    def start_moving_pixel_test(self):
        if self.is_streaming and self.stream_type == "moving":
            self.clear_screen()
            return
        self.is_streaming = False 
        self.stream_type = "moving" 
        self.status_label.config(text="상태: 커스텀 컬러 단일 픽셀 순차 이동 테스트 중...")
        self.gif_frames.clear()
        self.frame_duration = 0.08 
        for i in range(64):
            mask_frame = bytearray(64)
            mask_frame[i] = 255 
            self.gif_frames.append(mask_frame)
        self.is_streaming = True
        asyncio.run_coroutine_threadsafe(self.stream_gif(), self.ble_loop)

    # --- [신규] 물방울 번짐 애니메이션 ---
    def start_ripple_effect(self):
        if self.is_streaming and self.stream_type == "ripple":
            self.clear_screen()
            return
            
        self.is_streaming = False 
        self.stream_type = "ripple" 
        self.status_label.config(text="상태: 물결(Ripple) 퍼짐 애니메이션 재생 중...")
        self.gif_frames.clear()
        self.frame_duration = 0.05 
        
        frames_count = 30 # 한 사이클을 30프레임으로 부드럽게 나눔
        for t in range(frames_count):
            mask_frame = bytearray(64)
            for y in range(8):
                for x in range(8):
                    # 매트릭스 중앙(3.5, 3.5)으로부터의 거리 계산
                    dist = np.sqrt((x-3.5)**2 + (y-3.5)**2)
                    # 거리와 프레임 시간(t)에 비례하여 물결 파동(sin) 계산
                    phase = dist * 1.5 - (t / frames_count) * 2 * np.pi
                    val = int(127 * (np.sin(phase) + 1))
                    mask_frame[y * 8 + x] = val
            self.gif_frames.append(mask_frame)

        self.is_streaming = True
        asyncio.run_coroutine_threadsafe(self.stream_gif(), self.ble_loop)

    async def stream_gif(self):
        frame_idx = 0
        while self.is_streaming and self.client and self.client.is_connected:
            if not self.gif_frames:
                await asyncio.sleep(0.1)
                continue
            current_frame = self.gif_frames[frame_idx]
            
            if self.stream_type == "gif":
                payload = current_frame
            else:
                # scroll, ripple, checker, moving 애니메이션일 때 고유 지정 RGB 주입
                payload = bytearray(192)
                for i in range(64):
                    br = current_frame[i] / 255.0 # 0.0 ~ 1.0 비율
                    p = self.pixels[i]
                    payload[i*3]     = int(p['r'] * br)
                    payload[i*3 + 1] = int(p['g'] * br)
                    payload[i*3 + 2] = int(p['b'] * br)
            
            try:
                await self.client.write_gatt_char(CHARACTERISTIC_UUID, payload, response=False)
            except Exception as e:
                print(f"전송 에러: {e}")
                self.is_streaming = False
                break
                
            self.run_in_main_thread(lambda p=payload: self.sync_ui_with_frame(p))
            frame_idx = (frame_idx + 1) % len(self.gif_frames)
            await asyncio.sleep(self.frame_duration)

    def sync_ui_with_frame(self, frame_bytes):
        if self.is_streaming: 
            if self.stream_type == "gif":
                for i in range(64):
                    r, g, b = frame_bytes[i*3], frame_bytes[i*3+1], frame_bytes[i*3+2]
                    self.pixels[i]['r'], self.pixels[i]['g'], self.pixels[i]['b'] = r, g, b
                    self.pixels[i]['br'] = max(r, g, b) 
            else:
                for i in range(64):
                    r, g, b = frame_bytes[i*3], frame_bytes[i*3+1], frame_bytes[i*3+2]
                    self.pixels[i]['br'] = max(r, g, b)
            self.refresh_all_canvases()

    def run_in_main_thread(self, func):
        self.after(0, func)

if __name__ == "__main__":
    app = LedStreamerApp()
    app.mainloop()