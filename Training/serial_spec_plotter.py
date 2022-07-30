import cv2
import serial
import numpy as np

melSpec = np.zeros([40, 30])
resultMatrix = np.zeros([6, 30])

cnt = 0

with serial.Serial("COM6", 115200) as ser:
    while True:
        line = ser.readline()
        if line[:4] == b"spec":
            melColList = line[4:].decode().split("|")
            melCol = np.asarray(melColList)

            melSpec[:,0:29] = melSpec[:,1:30]
            melSpec[:,29] = melCol[:40]

            imgBuffer = np.zeros([40, 30, 3], dtype=np.uint8)

            imgBuffer[:,:,0] = ((melSpec - 80) / 160 * 255).astype(int)
            imgBuffer[:,:,1] = imgBuffer[:,:,0]
            imgBuffer[:,:,2] = imgBuffer[:,:,0]

            imgBuffer = cv2.resize(imgBuffer, (600, 800), interpolation=cv2.INTER_NEAREST)

            cv2.imshow("Spec", imgBuffer)
            cv2.waitKey(10)

            if cnt % 30 == 0:
                print(cnt)
            cnt += 1


        elif line[:4] == b"res:":
            resultLst = line[4:].decode().split("|")
            resultVec = np.asarray(resultLst)

            resultMatrix[:,0:29] = resultMatrix[:,1:30]
            resultMatrix[:,29] = resultVec[:6]

            imgBuffer = np.zeros([6, 30, 3], dtype=np.uint8)

            imgBuffer[:,:,0] = (resultMatrix * 255).astype(int)
            imgBuffer[:,:,1] = imgBuffer[:,:,0]
            imgBuffer[:,:,2] = imgBuffer[:,:,0]

            imgBuffer = cv2.resize(imgBuffer, (600, 240), interpolation=cv2.INTER_NEAREST)

            cv2.imshow("Result", imgBuffer)
            cv2.waitKey(10)

            if cnt % 30 == 0:
                print(cnt)
            cnt += 1

        else:
            print(line)