set dir_in=.
set dir_out=%dir_in%
set protoc="../../../contrib/google/protobuf/bin/protoc.exe"

%protoc% -I%dir_in% --cpp_out=%dir_out% ^
%dir_in%\Object.proto

%protoc% -I%dir_in% --js_out=import_style=commonjs,binary:. ^
%dir_in%\Object.proto

pause