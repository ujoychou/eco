set dir_in=.
set dir_out=%dir_in%
set protoc="../../../../../com/protobuf2.5.0/google/protobuf/bin/protoc.exe"

%protoc% -I%dir_in% --cpp_out=%dir_out% ^
%dir_in%\Monitor.proto

pause