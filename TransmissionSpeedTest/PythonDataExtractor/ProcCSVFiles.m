% Author: Trevor Sherrard
% Course: Mechatronics
% Date: 03/08/2020
% Filename: ProcCSVFiles.m

T = readtable("rawData\noFoil.csv")
T2 = readtable("rawData\Foil.csv")
testInd = 10

% grab binary streams
EncyptX_tx = cell2mat(T.EncryptedTxBinaryStreamForX(testInd))
EncyptY_tx = cell2mat(T.EncryptedTxBinaryStreamForY(testInd))
EncyptZ_tx = cell2mat(T.EncryptedTxBinaryStreamForZ(testInd))
EncyptX_rx = cell2mat(T.EncryptedRxBinaryStreamForX(testInd))
EncyptY_rx = cell2mat(T.EncryptedRxBinaryStreamForY(testInd))
EncyptZ_rx = cell2mat(T.EncryptedRxBinaryStreamForZ(testInd))

% trim excess quote characters
EncyptX_tx = EncyptX_tx(2:length(EncyptX_tx) - 1)
EncyptY_tx = EncyptY_tx(2:length(EncyptY_tx) - 1)
EncyptZ_tx = EncyptZ_tx(2:length(EncyptZ_tx) - 1)
EncyptX_rx = EncyptX_rx(2:length(EncyptX_rx) - 1)
EncyptY_rx = EncyptY_rx(2:length(EncyptY_rx) - 1)
EncyptZ_rx = EncyptZ_rx(2:length(EncyptZ_rx) - 1)

% grab total transmission times
TotalTx_X = T.TxTimeForX(testInd);
TotalTx_Y = T.TxTimeForY(testInd);
TotalTx_Z = T.TxTimeForZ(testInd);

% grab round trip times
ToFX = T.RoundTripTimeForX(testInd)*(1e-6);
ToFY = T.RoundTripTimeForY(testInd)*(1e-6);
ToFZ = T.RoundTripTimeForZ(testInd)*(1e-6);

for i = 2:length(EncyptX_tx)
    EncyptX_tx_num(i) = str2num(EncyptX_tx(i));
end
for i = 2:length(EncyptY_tx)
    EncyptY_tx_num(i) = str2num(EncyptY_tx(i));
end
for i = 2:length(EncyptZ_tx)
    EncyptZ_tx_num(i) = str2num(EncyptZ_tx(i));
end
for i = 2:length(EncyptX_rx)
    EncyptX_rx_num(i) = str2num(EncyptX_rx(i));
end
for i = 2:length(EncyptY_rx)
    EncyptY_rx_num(i) = str2num(EncyptY_rx(i));
end
for i = 2:length(EncyptZ_rx)
    EncyptZ_rx_num(i) = str2num(EncyptZ_rx(i));
end

% compute individual time axes for each tx data stream
individualT_x = TotalTx_X/numel(EncyptX_tx_num)*(1e-6);
t_x = 0:individualT_x:(numel(EncyptX_tx_num) - 1)*individualT_x;

individualT_y = TotalTx_Y/numel(EncyptY_tx_num)*(1e-6);
t_y = 0:individualT_y:(numel(EncyptY_tx_num) - 1)*individualT_y;

individualT_z = TotalTx_Z/numel(EncyptZ_tx_num)*(1e-6);
t_z = 0:individualT_z:(numel(EncyptZ_tx_num) - 1)*individualT_z;

% compute individual time axes for each rx data stream
individualT_x_rx = TotalTx_X/numel(EncyptX_rx_num)*(1e-6);
t_x_rx = 0:individualT_x_rx:(numel(EncyptX_rx_num) - 1)*individualT_x_rx;

individualT_y_rx = TotalTx_Y/numel(EncyptY_rx_num)*(1e-6);
t_y_rx = 0:individualT_y_rx:(numel(EncyptY_rx_num) - 1)*individualT_y_rx;

individualT_z_rx = TotalTx_X/numel(EncyptZ_rx_num)*(1e-6);
t_z_rx = 0:individualT_z_rx:(numel(EncyptZ_rx_num) - 1)*individualT_z_rx;

% plot tx signal waveforms

firstShift = TotalTx_X*(1e-6);
subplot(6,1,1)
stairs(t_x, EncyptX_tx_num,'r','linewidth',1)
title("Data Stream For Transmitted Encrypted Sensor Reading (X-Orientation)");
xlim([0 25*10^-3])

subplot(6,1,2)
stairs(t_y + firstShift, EncyptY_tx_num,'r','linewidth',1)
title("Data Stream For Transmitted Encrypted Sensor Reading (Y-Orientation)");
xlim([0 25*10^-3])

subplot(6,1,3)
stairs(t_z + 2*firstShift, EncyptZ_tx_num,'r','linewidth',1)
title("Data Stream For Transmitted Encrypted Sensor Reading (Z-Orientation)");
xlim([0 25*10^-3])

% plot rx signal waveforms
subplot(6,1,4)
stairs(t_y_rx + 2*firstShift + ToFX,EncyptY_rx_num,'b','linewidth',1)
title("Data Stream For Recieved Encrypted Sensor Reading (X-Orientation)");
xlim([0 25*10^-3])

subplot(6,1,5)
stairs(t_z_rx + 3.5*firstShift + ToFZ,EncyptZ_rx_num,'b','linewidth',1)
title("Data Stream For Recieved Encrypted Sensor Reading (Y-Orientation)");
xlim([0 25*10^-3])

subplot(6,1,6)
stairs(t_x_rx + 4.5*firstShift + ToFY,EncyptX_rx_num,'b','linewidth',1)
title("Data Stream For Recieved Encrypted Sensor Reading (Z-Orientation)");
xlim([0 25*10^-3])
sgtitle("TX and RX Data Streams For Encrypted BNO055 Sensor Data (No Foil)");

% calculate averages of round trip times
RoundTripXArr = mean(T.RoundTripTimeForX.*(1e-6));
RoundTripYArr = mean(T.RoundTripTimeForY.*(1e-6));
RoundTripZArr = mean(T.RoundTripTimeForZ.*(1e-6));

RoundTripXArr2 = mean(T2.RoundTripTimeForX.*(1e-6));
RoundTripYArr2 = mean(T2.RoundTripTimeForY.*(1e-6));
RoundTripZArr2 = mean(T2.RoundTripTimeForZ.*(1e-6));

% calculate averages for tx times
XnoFoil = mean(T.TxTimeForX.*(1e-6));
YnoFoil = mean(T.TxTimeForY.*(1e-6));
ZnoFoil = mean(T.TxTimeForZ.*(1e-6));

XFoil = mean(T2.TxTimeForX.*(1e-6))
YFoil = mean(T2.TxTimeForY.*(1e-6))
ZFoil = mean(T2.TxTimeForZ.*(1e-6))

% grab other timing metrics
dencryptNoFoil = mean(T.DencyptionTime.*(1e-6));
encryptNoFoil = mean(T.EncryptionTime.*(1e-6));
dencryptFoil = mean(T2.DencyptionTime.*(1e-6));
encryptFoil = mean(T2.EncryptionTime.*(1e-6));

foilPollTime = mean(T.SensorPollTime.*(1e-6));
nofoilPollTime = mean(T2.SensorPollTime.*(1e-6));





    