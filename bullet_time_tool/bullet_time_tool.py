import os
import sys
import re
import traceback
import numpy as np
import cv2
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import Qt

try:
    import av
    HAS_PYAV = True
except:
    HAS_PYAV = False


def natural_key(s: str):
    return [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", s)]

class VideoReader:
    def __init__(self, path, cache_all=False):
        self.path = path
        self.cache_all = cache_all
        self._cache = None
        self.cap = None
        self._ok = False
        self._frames = 0
        self._fps = 30
        self._w = 0
        self._h = 0
        self._init()

    def ok(self):
        return self._ok

    def fps(self):
        return self._fps

    def frame_count(self):
        return self._frames

    def size(self):
        return (self._w, self._h)

    def _init(self):
        if self.cache_all:
            cap = cv2.VideoCapture(self.path)
            if cap.isOpened():
                fps = cap.get(cv2.CAP_PROP_FPS) or 30
                frames = []
                while True:
                    ok, f = cap.read()
                    if not ok:
                        break
                    frames.append(f)
                cap.release()
                if len(frames):
                    h, w = frames[0].shape[:2]
                    self._cache = frames
                    self._fps = fps
                    self._frames = len(frames)
                    self._w, self._h = w, h
                    self._ok = True
                    return

        cap = cv2.VideoCapture(self.path)
        if cap.isOpened():
            fps = cap.get(cv2.CAP_PROP_FPS) or 30
            cnt = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
            w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
            h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            if cnt > 0:
                self.cap = cap
                self._fps = fps
                self._frames = cnt
                self._w, self._h = w, h
                self._ok = True
                return
        self._ok = False

    def read_frame(self, idx):
        idx = int(np.clip(idx, 0, self._frames - 1))
        if self._cache is not None:
            return self._cache[idx]
        if self.cap:
            self.cap.set(cv2.CAP_PROP_POS_FRAMES, idx)
            ok, f = self.cap.read()
            return f if ok else None
        return None

class VideoWriter:
    def __init__(self, path, fps, size):
        self.writer = cv2.VideoWriter(
            path,
            cv2.VideoWriter_fourcc(*"mp4v"),
            fps,
            size
        )
    def write(self, frame):
        self.writer.write(frame)

    def release(self):
        self.writer.release()

class WorkerThread(QtCore.QThread):
    progress = QtCore.pyqtSignal(int)
    finished = QtCore.pyqtSignal(str)
    error = QtCore.pyqtSignal(str)

    def __init__(self, params):
        super().__init__()
        self.params = params
        self.rotation = params.get("rotation", 0)

    def apply_rotation(self, f):
        if self.rotation == 90:
            return cv2.rotate(f, cv2.ROTATE_90_CLOCKWISE)
        if self.rotation == 180:
            return cv2.rotate(f, cv2.ROTATE_180)
        if self.rotation == 270:
            return cv2.rotate(f, cv2.ROTATE_90_COUNTERCLOCKWISE)
        return f

    def run(self):
        try:
            fps = self.params["fps"]
            main_path = self.params["main_path"]
            selected = self.params["selected"]
            target_index = self.params["target_index"]
            pre_s = self.params["pre_s"]
            post_s = self.params["post_s"]
            hold_s = self.params["hold_s"]
            out_path = self.params["out_path"]

            main = VideoReader(main_path, cache_all=True)
            if not main._ok:
                raise Exception("主影片無法讀取")

            w, h = main._w, main._h
            if self.rotation in (90, 270):
                out_w, out_h = h, w
            else:
                out_w, out_h = w, h

            writer = VideoWriter(out_path, fps, (out_w, out_h))

            target_time = target_index / fps
            pre_frames = int(pre_s * fps)
            post_frames = int(post_s * fps)
            hold_frames = max(1, int(hold_s * fps))

            total = pre_frames + post_frames + hold_frames * len(selected)
            counter = 0

            # Pre-roll
            start = max(0, target_index - pre_frames)
            for i in range(start, target_index):
                f = main.read_frame(i)
                writer.write(self.apply_rotation(f))
                counter += 1
                self.progress.emit(int(counter/total*100))

            # Bullet time frames
            for _, path in selected:
                vr = VideoReader(path)
                t_idx = int(vr._fps * target_time)
                f_bt = vr.read_frame(t_idx)
                f_bt = self.apply_rotation(f_bt)
                for _ in range(hold_frames):
                    writer.write(f_bt)
                    counter += 1
                    self.progress.emit(int(counter/total*100))

            # Post-roll
            end = min(main._frames - 1, target_index + post_frames)
            for i in range(target_index, end + 1):
                f = main.read_frame(i)
                writer.write(self.apply_rotation(f))
                counter += 1
                self.progress.emit(int(counter/total*100))

            writer.release()
            self.finished.emit(out_path)

        except Exception:
            self.error.emit(traceback.format_exc())

class BulletTimeApp(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("子彈時間工具 (旋轉版)")
        self.resize(1280, 800)

        self.current_folder = None
        self.main_reader = None
        self.current_main_name = None
        self.current_frame_index = 0

        self.config = self.load_config()
        self.build_ui()

    def build_ui(self):
        central = QtWidgets.QWidget()
        self.setCentralWidget(central)
        layout = QtWidgets.QHBoxLayout(central)

        left = QtWidgets.QVBoxLayout()
        btn = QtWidgets.QPushButton("選擇影片資料夾…")
        btn.clicked.connect(self.choose_folder)
        left.addWidget(btn)

        self.list_videos = QtWidgets.QListWidget()
        self.list_videos.itemClicked.connect(self.on_video_clicked)
        left.addWidget(self.list_videos, 1)

        right = QtWidgets.QVBoxLayout()
        self.label_video = QtWidgets.QLabel("尚未載入影片")
        self.label_video.setAlignment(Qt.AlignCenter)
        self.label_video.setMinimumHeight(400)
        right.addWidget(self.label_video)

        sl = QtWidgets.QHBoxLayout()
        self.slider = QtWidgets.QSlider(Qt.Horizontal)
        self.slider.valueChanged.connect(self.on_seek)
        self.lbl_info = QtWidgets.QLabel("0/0")
        sl.addWidget(self.slider, 1)
        sl.addWidget(self.lbl_info)
        right.addLayout(sl)

        r_rot = QtWidgets.QHBoxLayout()
        r_rot.addWidget(QtWidgets.QLabel("輸出旋轉角度："))
        self.combo_rot = QtWidgets.QComboBox()
        self.combo_rot.addItems(["0", "90", "180", "270"])
        r_rot.addWidget(self.combo_rot)
        right.addLayout(r_rot)

        self.list_other = QtWidgets.QListWidget()
        self.list_other.setSelectionMode(QtWidgets.QAbstractItemView.NoSelection)
        right.addWidget(self.list_other, 1)

        self.btn_export = QtWidgets.QPushButton("輸出影片")
        self.btn_export.clicked.connect(self.export_video)
        right.addWidget(self.btn_export)

        layout.addLayout(left, 3)
        layout.addLayout(right, 7)

        self.progress = QtWidgets.QProgressBar()
        self.progress.hide()
        self.statusBar().addPermanentWidget(self.progress)

    # -------------------------------------------------------
    # Folder loading （載入資料夾 + 排序 + 填入左側列表）
    # -------------------------------------------------------
    def choose_folder(self):
        d = QtWidgets.QFileDialog.getExistingDirectory(
            self, "選擇影片資料夾",
            self.config.get("default_folder", "")
        )
        if not d:
            return

        self.current_folder = d
        self.load_video_list()

    def load_video_list(self):
        self.list_videos.clear()
        self.list_other.clear()
        self.main_reader = None
        self.current_main_name = None

        if not self.current_folder:
            return

        files = [
            f for f in os.listdir(self.current_folder)
            if f.lower().endswith(".mp4")
        ]

        # ✔ 自然排序
        files.sort(key=natural_key)

        self.video_list = files

        for f in files:
            self.list_videos.addItem(f)

        # ✔ 如果參數檔指定了預設主影片，則自動選取
        default_main = self.config.get("default_main", "")

        if default_main in files:
            items = self.list_videos.findItems(default_main, Qt.MatchExactly)
            if items:
                self.list_videos.setCurrentItem(items[0])
                self.on_video_clicked(items[0])
                return

        # ✔ 否則選第一個
        if files:
            first = self.list_videos.item(0)
            self.list_videos.setCurrentItem(first)
            self.on_video_clicked(first)

    # -------------------------------------------------------
    # 點選左側影片 → 主影片
    # -------------------------------------------------------
    def on_video_clicked(self, item):
        if not item:
            return

        name = item.text()
        self.current_main_name = name

        path = os.path.join(self.current_folder, name)

        # ✔ 主影片使用快取版本（不卡）
        self.main_reader = VideoReader(path, cache_all=True)
        if not self.main_reader._ok:
            self.label_video.setText("無法讀取影片")
            return

        # 更新 slider
        self.slider.blockSignals(True)
        self.slider.setMaximum(self.main_reader._frames - 1)
        self.slider.setValue(0)
        self.slider.blockSignals(False)

        self.current_frame_index = 0
        f = self.main_reader.read_frame(0)
        self.update_video(f)
        self.update_frame_label()

        # ✔ 自動生成 bullet time 來源清單（旋轉列表）
        self.build_rotate_list(name)

    def build_rotate_list(self, main_name):
        self.list_other.clear()

        if main_name not in self.video_list:
            return

        # 主影片在列表中的 index
        idx = self.video_list.index(main_name)

        # ✔ 自動依序：idx, idx+1, …, end, 0, 1, …（循環）
        ordered = []
        N = len(self.video_list)
        for i in range(N):
            j = (idx + i) % N
            ordered.append(self.video_list[j])

        # 第一個永遠是主影片，不放進來源
        sources = ordered[1:]

        self.auto_sources = sources  # 給 export 使用

        for f in sources:
            item = QtWidgets.QListWidgetItem(f)
            item.setCheckState(Qt.Checked)
            self.list_other.addItem(item)

    # -------------------------------------------------------
    # Slider → 換畫面（使用快取不卡）
    # -------------------------------------------------------
    def on_seek(self, v):
        if not self.main_reader:
            return
        self.current_frame_index = v
        f = self.main_reader.read_frame(v)
        self.update_video(f)
        self.update_frame_label()

    def update_video(self, frame):
        if frame is None:
            self.label_video.setText("無影像")
            return

        h, w = frame.shape[:2]
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        qimg = QtGui.QImage(rgb.data, w, h, w*3, QtGui.QImage.Format_RGB888)
        pix = QtGui.QPixmap.fromImage(qimg)
        pix = pix.scaled(self.label_video.size(), Qt.KeepAspectRatio, Qt.SmoothTransformation)
        self.label_video.setPixmap(pix)

    def update_frame_label(self):
        if not self.main_reader:
            self.lbl_info.setText("0 / 0")
            return
        idx = self.current_frame_index
        total = self.main_reader._frames
        t = idx / self.main_reader._fps
        self.lbl_info.setText(f"{idx+1} / {total}  ({t:.3f}s)")

    def export_video(self):
        if not self.main_reader:
            QtWidgets.QMessageBox.warning(self, "錯誤", "請先載入影片")
            return

        rotation = int(self.combo_rot.currentText())

        selected = []
        for name in self.auto_sources:
            path = os.path.join(self.current_folder, name)
            selected.append((name, path))

        out_path, _ = QtWidgets.QFileDialog.getSaveFileName(
            self, "輸出影片",
            os.path.join(self.current_folder, "bullet_time_rotated.mp4"),
            "MP4 (*.mp4)"
        )

        if not out_path:
            return

        params = {
            "fps": self.main_reader.fps(),
            "main_path": os.path.join(self.current_folder, self.current_main_name),
            "selected": selected,
            "target_index": self.current_frame_index,
            "pre_s": self.config.get("pre_seconds", 1.0),
            "post_s": self.config.get("post_seconds", 1.0),
            "hold_s": self.config.get("hold_seconds", 0.15),
            "rotation": rotation,
            "out_path": out_path
        }

        self.worker = WorkerThread(params)
        self.worker.progress.connect(self.progress.setValue)
        self.worker.finished.connect(self.on_export_done)
        self.worker.error.connect(self.on_export_error)

        self.progress.setValue(0)
        self.progress.show()
        self.btn_export.setEnabled(False)
        self.worker.start()

     # -------------------------------------------------------
    # 子彈時間輸出完成
    # -------------------------------------------------------
    def on_export_done(self, out_path):
        self.progress.hide()
        self.btn_export.setEnabled(True)
        self.statusBar().showMessage("輸出完成")

        QtWidgets.QMessageBox.information(
            self, "完成",
            f"影片輸出成功：\n{out_path}"
        )

        # 如果你希望自動打開輸出資料夾，也可以加上：
        # import os
        # os.startfile(os.path.dirname(out_path))


    # -------------------------------------------------------
    # 子彈時間輸出錯誤
    # -------------------------------------------------------
    def on_export_error(self, msg):
        self.progress.hide()
        self.btn_export.setEnabled(True)
        self.statusBar().showMessage("輸出失敗")

        QtWidgets.QMessageBox.critical(
            self, "錯誤",
            f"在產生影片時發生錯誤：\n\n{msg}"
        ) 

    # -------------------------------------------------------
    # JSON 參數檔：讀取/保存
    # -------------------------------------------------------
    def load_config(self):
        import json

        cfg_path = os.path.join(os.path.dirname(__file__), "config.json")

        default_cfg = {
            "default_folder": "",
            "default_main": "",
            "pre_seconds": 1.0,
            "post_seconds": 1.0,
            "hold_seconds": 0.15
        }

        if not os.path.exists(cfg_path):
            return default_cfg

        try:
            with open(cfg_path, "r", encoding="utf-8") as f:
                user_cfg = json.load(f)

            # 補上缺少的 key
            for k, v in default_cfg.items():
                if k not in user_cfg:
                    user_cfg[k] = v

            return user_cfg

        except Exception:
            return default_cfg

    def save_config(self):
        import json
        cfg_path = os.path.join(os.path.dirname(__file__), "config.json")

        try:
            with open(cfg_path, "w", encoding="utf-8") as f:
                json.dump(self.config, f, indent=4, ensure_ascii=False)
        except Exception as e:
            print("❌ 無法寫入 config.json:", e)


def main():
    app = QtWidgets.QApplication(sys.argv)
    w = BulletTimeApp()
    w.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
