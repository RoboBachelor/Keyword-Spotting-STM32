import cv2
import serial
import numpy as np

melSpec = np.zeros([6, 30])

cnt = 0

with serial.Serial("COM6", 115200) as ser:
    while True:
        line = ser.readline()
        if line[:4] == b"res:":
            melColList = line[4:].decode().split("|")
            melCol = np.asarray(melColList)

            melSpec[:,0:29] = melSpec[:,1:30]
            melSpec[:,29] = melCol[:6]

            imgBuffer = np.zeros([6, 30, 3], dtype=np.uint8)

            imgBuffer[:,:,0] = (melSpec * 255).astype(int)
            imgBuffer[:,:,1] = imgBuffer[:,:,0]
            imgBuffer[:,:,2] = imgBuffer[:,:,0]

            imgBuffer = cv2.resize(imgBuffer, (600, 240), interpolation=cv2.INTER_NEAREST)

            cv2.imshow("Spec", imgBuffer)
            cv2.waitKey(10)

            if cnt % 30 == 0:
                print(cnt)
            cnt += 1

        else:
            print(line)