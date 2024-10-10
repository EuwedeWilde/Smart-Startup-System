import serial
import subprocess
import re
from pymongo import MongoClient
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CONTROLLER_DIR = os.path.dirname(SCRIPT_DIR)
PROJECT_ROOT = os.path.dirname(CONTROLLER_DIR)

SERIAL_PORT = 'COM7'  
SKETCH_PATH = os.path.join(PROJECT_ROOT, 'controller', 'sketches', 'dev_esp32', 'dev_esp32.ino')
CONFIG_PATH = os.path.join(PROJECT_ROOT, 'controller', 'sketches', 'dev_esp32', 'config.h')
BOARD = 'esp32:esp32:esp32'
UPLOAD_PORT = 'COM7'

# MongoDB connection settings
MONGODB_URI = 'mongodb://127.0.0.1:27017/local'
DB_NAME = 'local'
COLLECTION_NAME = 'device_info'

def get_mac_address():
    print("Waiting for MAC address...")
    ser = serial.Serial(SERIAL_PORT, 115200, timeout=30)
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

def get_mongodb_connection():
    client = MongoClient(MONGODB_URI)
    db = client[DB_NAME]
    collection = db[COLLECTION_NAME]
    return collection

def get_device_id(mac):
    collection = get_mongodb_connection()
    device = collection.find_one({'dMac': mac})
    return device['did'] if device else None

def assign_device_id(mac):
    collection = get_mongodb_connection()
    highest_id_doc = collection.find_one(sort=[('did', -1)])
    new_id = 1 if not highest_id_doc else highest_id_doc.get('did', 0) + 1
    collection.insert_one({'dMac': mac, 'did': new_id})
    return new_id

def update_device_id(new_id):
    print(f"Attempting to open file: {CONFIG_PATH}")
    with open(CONFIG_PATH, 'r') as file:
        content = file.read()
    
    updated_content = re.sub(r'#define DEVICE_ID \d+', f'#define DEVICE_ID {new_id}', content)
    
    with open(CONFIG_PATH, 'w') as file:
        file.write(updated_content)
    
    print(f"Updated DEVICE_ID to {new_id} in {CONFIG_PATH}")

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

    device_id = get_device_id(mac)

    if device_id:
        print(f"Device with MAC {mac} already has ID {device_id}")
    else:
        device_id = assign_device_id(mac)
        print(f"New device ID {device_id} assigned to MAC {mac}")

    # Always update the DEVICE_ID in config.h
    update_device_id(device_id)
    
    input("Press Enter to start uploading the sketch...")
    upload_sketch()

    input("Press Enter to exit...")

if __name__ == "__main__":
    print("Starting the upload process...")
    main()
    print("Upload process completed.")
