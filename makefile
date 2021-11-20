all: echo-server echo-client

echo-server:
	g++ -g -Wall -o echo-server echo-server.cpp -pthread
	
echo-client:
	g++ -g -Wall -o echo-client echo-client.cpp -pthread

clean:
	rm -f echo-server
	rm -f echo-client
	rm -f *.o
