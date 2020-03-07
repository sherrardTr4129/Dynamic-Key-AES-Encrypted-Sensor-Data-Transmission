# Author: Trevor Sherrard
# Course: Mechatronics Term Project
# Date: 03/07/2020
# Filename: generateDataCSV

import csv

filename = "Foil.csv"
csvFile = open(filename, 'w')
csvWriter = csv.writer(csvFile, delimiter=",")
csvWriter.writerow(["Non-Encrypted Tx", "Encrypted Tx Binary Stream For X", "Encrypted Tx Binary Stream For Y",
                    "Encrypted Tx Binary Stream For Z", "Encrypted Rx Binary Stream For X", "Encrypted Rx Binary Stream For Y",
                    "Encrypted Rx Binary Stream For Z", "Non-Encrypted Rx", "Tx time for X", "Tx time for Y", "Tx time for Z",
                    "Round Trip Time For X", "Round Trip Time For Y", "Round Trip Time For Z", "Encryption Time", "Sensor Poll Time",
                    "Dencyption Time"])
for line in open("Foil.txt", 'r'):
    splitLine = line.split("!")
    if(len(splitLine) > 2):
        sentDataReadble = splitLine[0]
        x_tx = splitLine[1]
        y_tx = splitLine[2]
        z_tx = splitLine[3]
        x_rx = splitLine[4]
        y_rx = splitLine[5]
        z_rx = splitLine[6]
        rxReadable = splitLine[7]
        
        totalTxTimeX = splitLine[8].split(',')[0]
        totalTxTimeY = splitLine[8].split(',')[1]
        totalTxTimeZ = splitLine[8].split(',')[2]
        totalRoundTripTimeX = splitLine[9].split(',')[0]
        totalRoundTripTimeY = splitLine[9].split(',')[1]
        totalRoundTripTimeZ = splitLine[9].split(',')[2]
        
        encryptTotal = splitLine[10]
        pollTotal = splitLine[11]
        print(pollTotal)
        dencryptTotal = splitLine[12].split("\"")[-1]

        
        csvWriter.writerow([sentDataReadble, x_tx, y_tx, z_tx, x_rx, y_rx, z_rx, rxReadable,
                            totalTxTimeX, totalTxTimeY, totalTxTimeZ, totalRoundTripTimeX, totalRoundTripTimeY,
                            totalRoundTripTimeZ, encryptTotal, pollTotal, dencryptTotal])
csvFile.close()
