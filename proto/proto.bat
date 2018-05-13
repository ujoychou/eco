set dir_in=.
set dir_out=%dir_in%

"../../../contrib/google/protobuf/bin/protoc.exe" -I%dir_in% --cpp_out=%dir_out% ^
%dir_in%\Object.proto

pause