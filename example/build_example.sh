cd proto && \
./gen_proto.sh && \
cp simple_example_server_bak.cc simple_example_server.cc && \
cd ..
g++ -c proto/simple_example_server.cc -I../ -I./proto
g++ simple_example_server.o test_simple_example_server.cc -o test_simple_example_server -I../ -I./proto
g++ test_simple_example_client.cc -o test_simple_example_client -I../ -I./proto
