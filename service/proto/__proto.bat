set proto_dir=.
set cpp_out_dir=%proto_dir%

"../../../../contrib/google/protobuf/bin/protoc.exe" --proto_path=%proto_dir% --cpp_out=%cpp_out_dir% ^
%proto_dir%\Service.proto ^
%proto_dir%\Error.proto
pause