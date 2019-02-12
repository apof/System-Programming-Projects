mirror_system: mirror_initiator.cc mirror_server.cc content_server.cc
	g++ -o initiator mirror_initiator.cc
	g++ -o mirror_server mirror_server.cc -lpthread
	g++ -o content_server content_server.cc
clean:

	rm initiator
	rm mirror_server
	rm content_server

