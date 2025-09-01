M5StickC Plus Bluetooth Mouse Jiggler
<img width="512" height="512" alt="Mouse Jiggler" src="https://github.com/user-attachments/assets/e4b2b693-f91e-4e53-a023-eecac9d788bb" />


This project turns an M5StickC Plus into a Bluetooth mouse jiggler using the BleMouse library (Specifically > [https://github.com/sirfragles/ESP32-BLE-Mouse/tree/dev]).
It prevents your computer from going idle or locking by sending small mouse movements at configurable intervals.

⸻

✨ Features
	•	Toggle jiggle ON/OFF with BtnA
	•	Change jiggle interval with BtnB (5s → 15s → 30s → 60s → 5min → 10min)
	•	Change jiggle distance with long press on BtnA (5px → 10px → 25px → 50px)
	•	OLED display UI shows:
	•	Jiggle status (ON/OFF, color coded)
	•	Current interval (seconds)
	•	Current distance (pixels)
	•	Progress bar until next jiggle
	•	Screen sleep after 30 seconds of no interaction
	•	Connects to any Bluetooth-capable host as a HID mouse

⸻

🔧 Hardware Requirements
	•	M5StickC Plus
	•	USB-C cable for programming

⸻

📦 Software Requirements
	•	Arduino IDE or PlatformIO
	•	Install the following libraries:
	•	M5StickCPlus (M5StickCPlus.h)
	•	ESP32 BLE Mouse (BleMouse.h) {Specifically > [https://github.com/sirfragles/ESP32-BLE-Mouse/tree/dev]}

⸻

🚀 Usage
	1.	Flash the firmware to your M5StickC Plus.
	2.	Power it on — it will advertise as a Bluetooth Mouse.
	3.	Pair it with your computer (first-time only).
	4.	Use buttons to control jiggle behavior:
	•	BtnA (short press): Toggle jiggle ON/OFF
	•	BtnB (short press): Cycle through jiggle intervals
	•	BtnA (long press >1s): Cycle through jiggle distances

⸻

📊 Display Layout
	•	Top line: Jiggle status (ON = green, OFF = red)
	•	Second line: Interval in seconds
	•	Third line: Jiggle distance in pixels
	•	Bottom bar: Progress bar showing time until next jiggle

⸻

⚠️ Notes
	•	The jiggle only works when connected to a Bluetooth host.
	•	Mouse movement is subtle and won’t interfere with normal use.
	•	Useful for preventing screensavers, auto-lock, or idle states.

⸻

📜 License

MIT License – feel free to modify and use.
