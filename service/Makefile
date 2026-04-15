.PHONY: compile upload monitor run

PORT ?= /dev/ttyUSB0
FQBN = esp32:esp32:esp32
CLI = arduino-cli
SKETCH = .

compile:
	$(CLI) compile --clean --fqbn $(FQBN) $(SKETCH)

upload: compile
	$(CLI) upload -p $(PORT) --fqbn $(FQBN) $(SKETCH)

monitor:
	$(CLI) monitor -p $(PORT) -c baudrate=115200

run: upload monitor
