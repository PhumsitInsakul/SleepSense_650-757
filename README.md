# SleepSense_650-757
คุณภาพการนอนด้วยการวัดมลพิษทางเสียง และแสง
 
 ## Table of Contents
  1. Hardware
  2. Firmware
  3. Frontend, Database
  4. Testing

### 1. Hardware
  - Schematic:
    - ![สกรีนช็อต 2023-11-10 074534](https://github.com/PhumsitInsakul/SleepSense/assets/96218618/4f9650d4-ce29-4e75-9ab3-1f3849133c65)
  - PCB:
    - ![สกรีนช็อต 2023-11-10 074701](https://github.com/PhumsitInsakul/SleepSense/assets/96218618/5c3695c2-100a-42cc-b50e-f31456f7cad5)
  - อุปกรณ์:
    - ![สกรีนช็อต 2023-11-10 074840](https://github.com/PhumsitInsakul/SleepSense/assets/96218618/36e5052a-2643-48c6-921a-912c502f803b)
    - ![S__20078596](https://github.com/PhumsitInsakul/SleepSense/assets/96218618/4c62be14-9547-4f50-9765-bc80eb0bb852)





### 2. Firmware
- Flowchart:
  - ![Flowcharts (4)](https://github.com/PhumsitInsakul/SleepSense_650-757/assets/96218618/54ea9b38-0cbd-4398-a512-3b6d75256110)


-  มีการควบคุมการทำงานแบบคาบเวลาด้วย millis()
-  มีการเปิดการใช้งาน Watchdog timer เพื่อป้องกันการค้าง และมีการเปิดใช้งาน esp32 light sleep mode เมื่อ sensor is off

### 3. Frontend
- Work flow:
  -  ![สกรีนช็อต 2023-11-10 075635](https://github.com/PhumsitInsakul/SleepSense/assets/96218618/d4eee81b-6aa2-43c4-be00-16753ebbad52)
- Frontend using Arduino IoT Cloud:
  - ![image](https://github.com/PhumsitInsakul/SleepSense/assets/96218618/6efddce5-3bd2-41c2-a171-5929cc54aebc)
- Database using Google Sheets:
  - ![สกรีนช็อต 2023-11-10 075039](https://github.com/PhumsitInsakul/SleepSense/assets/96218618/a280f4fe-5241-4ce5-b0bc-440c34cea9f2)



### 4. Testing
https://github.com/PhumsitInsakul/SleepSense











