set dir_in=.
set dir_out=%dir_in%
set protoc="../../../../com/protobuf3.5.1/google/protobuf/bin/protoc.exe"

%protoc% -I%dir_in% --cpp_out=%dir_out% ^
%dir_in%\Eco.proto

%protoc% -I%dir_in% --js_out=import_style=commonjs,binary:. ^
%dir_in%\Eco.proto

pause