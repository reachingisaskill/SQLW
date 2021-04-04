
.open 'database.db'

CREATE TABLE "Devices"
(
  DeviceIndex INTEGER PRIMARY KEY,
  DeviceIdentifier INTEGER UNIQUE not NULL,
  DeviceType TEXT not NULL,
  DeviceName TEXT not NULL,
  DeviceDescription TEXT not NULL
);


INSERT INTO Devices( DeviceIndex, DeviceIdentifier, DeviceType, DeviceName, DeviceDescription )
  VALUES
    ( 0, 0, 'master', 'Master', '' ),
    ( 1, 20, 'door', 'TestDoor2', '' ),
    ( 2, 30, 'door', 'TestDoor3', '' ),
    ( 3, 40, 'door', 'TestDoor4', '' ),
    ( 4, 50, 'door', 'TestDoor5', '' ),
    ( 5, 60, 'door', 'TestDoor6', '' ),
    ( 6, 70, 'door', 'TestDoor7', '' ),
    ( 7, 80, 'door', 'TestDoor8', '' ),
    ( 8, 42, 'door', 'PrimaryEntrance', 'Front door device for visitors and employees' );

