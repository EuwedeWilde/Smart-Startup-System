import serial
import json
import subprocess

SERIAL_PORT = 'COM7'  
SKETCH_PATH = 'controller/sketches/dev_esp32/dev_esp32.ino'
BOARD = 'esp32:esp32:esp32'
UPLOAD_PORT = 'COM7'

def get_mac_address():
    print("Waiting for MAC address...")
    ser = serial.Serial(SERIAL_PORT, 115200, timeout=30)  # Increased timeout
    try:
        while True:
            line = ser.readline().decode('utf-8').strip()
            if line.startswith("MAC:["):
                mac = line[5:-1]
                return mac
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    finally:
        ser.close()
    return None

def load_mac_database(filepath='mac_ids.json'):
  try:
    with open(filepath, 'r') as file:
      return json.load(file)
  except FileNotFoundError:
    return {}

def save_mac_database(data, filepath='mac_ids.json'):
  with open(filepath, 'w') as file:
    json.dump(data, file, indent=2)

def assign_device_id(mac, mac_db):
  if mac in mac_db:
    return None
  else:
    new_id = len(mac_db) + 1
    mac_db[mac] = new_id
    return new_id

def upload_sketch():
  compile_cmd = f'arduino-cli compile --fqbn {BOARD} {SKETCH_PATH}'
  upload_cmd = f'arduino-cli upload -p {UPLOAD_PORT} --fqbn {BOARD} {SKETCH_PATH}'

  try:
    print("Compiling the sketch...")
    subprocess.run(compile_cmd, shell=True, check=True)
    print("Uploading the sketch...")
    subprocess.run(upload_cmd, shell=True, check=True)
    print("Upload successful!")
  except subprocess.CalledProcessError as e:
    print(f"An error occurred during the process: {e}")

def main():
    input("Press Enter to get device MAC...")
    
    mac = get_mac_address()
    if not mac:
        print("Failed to retrieve MAC address. Please check the connection and try again.")
        return

    print(f"Retrieved MAC address: {mac}")
    
    input("Restart device in upload mode and press Enter to continue...")

    mac_db = load_mac_database()

    if mac in mac_db:
        print(f"Device with MAC {mac} already has ID {mac_db[mac]}")
    else:
        new_id = assign_device_id(mac, mac_db)
        if new_id:
            print(f"New device ID {new_id} assigned to MAC {mac}")
            save_mac_database(mac_db)
            
            input("Press Enter to start uploading the sketch...")
            upload_sketch()
        else:
            print(f"Device with MAC {mac} already registered")

    input("Press Enter to exit...")

if __name__ == "__main__":
    print("Starting the upload process...")
    main()
    print("Upload process completed.")
