set proto_dir=.
set cpp_out_dir=%proto_dir%

"../../../contrib/google/protobuf/bin/protoc.exe" -I%proto_dir% --cpp_out=%cpp_out_dir% ^
%proto_dir%\Object.proto

pause