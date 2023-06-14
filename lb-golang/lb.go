package main

import (
	"fmt"
	"net"
	"os"
	"sort"
	"time"
)

const TCP = "tcp"
const LB_PORT = ":20000"
const IDLE_TIMEOUT = 15 * time.Second // The timeout for idle connections

type MyLoadBalancer struct {
	servers []*Server
}

func (lb *MyLoadBalancer) bindConnection(conn net.Conn) *Server {
	// Select a server to forward the data to.
	// Apply Least Connections algorithm

	sort.Slice(lb.servers, func(i, j int) bool {
		// Find the server with the least number of connections
		// If there are multiple servers with the same number of connections, select the first one
		return len(lb.servers[i].clients) < len(lb.servers[j].clients)
	})
	lb.servers[0].clients = append(lb.servers[0].clients, conn)

	// Print the connection information
	fmt.Printf("\n%s -> %s\n", conn.RemoteAddr(), lb.servers[0].this.RemoteAddr())
	fmt.Println("Now the server manages the following connections:")
	for _, client := range lb.servers[0].clients {
		fmt.Println("- ", client.RemoteAddr().String())
	}
	return lb.servers[0]
}

func (lb *MyLoadBalancer) remove(server *Server) []net.Conn {
	// Remove the server from the list
	for i, s := range lb.servers {
		if s == server {
			lb.servers = append(lb.servers[:i], lb.servers[i+1:]...)
			break
		}
	}
	return server.clients
}

func (lb *MyLoadBalancer) healthCheck(server *Server, done <-chan struct{}) {
	
	for {
		// Run the health check every IDLE_TIMEOUT
		time.Sleep(IDLE_TIMEOUT)
		
		// Send a health check message to the server
		server.this.SetReadDeadline(time.Now().Add(IDLE_TIMEOUT))
		server.this.Write([]byte("HEALTH CHECK"))

		// Wait for the response
		select {
		case <-done:
			server.this.SetDeadline(time.Time{})
			fmt.Println("The server is alive:", server.this.RemoteAddr())
			fmt.Println()
		default:
		}
	}
}

func (lb *MyLoadBalancer) handleServer(server *Server, done chan<- struct{}) {
	buffer := make([]byte, 1024)

	for {
		// Receive data from the server
		nread, err := server.this.Read(buffer)
		if err != nil { 
			// If failed to read
			switch err := err.(type) {
			case net.Error:
				if err.Timeout() {
					// If connection timed out, remove the server from the list
					fmt.Println("The server is dead", server.this.RemoteAddr())
					orphans := lb.remove(server)

					// Move the orphan connections to another server
					for _, orphan := range orphans {
						fmt.Println("Moving connection", orphan.RemoteAddr(), "to another server")
						lb.bindConnection(orphan)
					}
					fmt.Println()
				}
			}
		}

		if nread > 0 {
			// Forward the data to the client
			fmt.Println("Received", nread, "bytes from server:", string(buffer[:nread]))
			for _, client := range server.clients {
				client.Write(buffer[:nread])
			}
			fmt.Println()
			done <- struct{}{}
		}
	}	
}

func (lb *MyLoadBalancer) forwardToServer(data []byte, server *Server) {
	// Forward the data to the server
	fmt.Println("Forwarding data to server", server.this.RemoteAddr().String())
	server.this.Write(data)
}

func (lb *MyLoadBalancer) handleClient(conn net.Conn, server *Server) {
	// Print the client's network address
	fmt.Println("\nConnection from", conn.RemoteAddr().String())

	buffer := make([]byte, 1024)
	
	for {
		// Receive data from the client
		nread, err := conn.Read(buffer)

		if err != nil {
			// If failed to read
			switch err := err.(type) {
			case net.Error:
				if err.Timeout() {
					// If connection timed out, remove the connection from the server
					fmt.Println("\nTimeout:", err)
					server.remove(conn)
					return
				} else {
					fmt.Println("Network error:", err)
					return
				}
			default:
				fmt.Println("Error reading:", err.Error())
			}
			return
		}
		// If succeeded to read
		// Refresh the idle deadline
		conn.SetDeadline(time.Now().Add(IDLE_TIMEOUT))
		lb.forwardToServer(buffer[:nread], server)

		fmt.Println("Received", nread, "bytes from client:", string(buffer[:nread]))
		fmt.Println("Deadline refreshed for connection", conn.RemoteAddr().String())
		fmt.Println()
	}
}

type Server struct {
	this net.Conn
	clients []net.Conn
}

func (server *Server) remove(conn net.Conn) {
	// Remove the connection from the server

	for i, client := range server.clients {
		if conn == client {
			fmt.Println("Disconnect", conn.RemoteAddr(), "from server", server.this.RemoteAddr())
			server.clients = append(server.clients[:i], server.clients[i+1:]...)
		}
	}
	conn.Close()
}

func main() {
	if len(os.Args) < 3 || len(os.Args)%2 != 1{
		// Get the IP address of the server from the command line arguments.
		// Number of arguments must be 2: the server IP address and the port number.
		fmt.Println("Usage: ", os.Args[0], " <server IP address> <server Port number>")
		os.Exit(1)
	}
	// initialize the load balancer
	lb := MyLoadBalancer{}

	// Store each server IP address and port number in the load balancer.
	for i := 1; i < len(os.Args); i += 2 {
		connFromServer, err := net.Dial(TCP, os.Args[i] + ":" + os.Args[i+1])

		if err != nil {
			fmt.Println("Error connecting to server:", err.Error())
			os.Exit(1)
		}
		fmt.Println("Connection from:", connFromServer.RemoteAddr().String())

		server := &Server{connFromServer, nil}
		lb.servers = append(lb.servers, server)
		defer connFromServer.Close()

		done := make(chan struct{})

		// Health check
		go lb.healthCheck(server, done)

		// Handle the connection from the server
		go lb.handleServer(server, done)
	}

	// Listen for incoming connections.
	listener, err := net.Listen(TCP, LB_PORT)
	fmt.Println("\nLoad balancer listening on port", LB_PORT)

	if err != nil {
		// If there is an error during the listen call, print the error and exit.
		fmt.Println("Error listening:", err.Error())
		os.Exit(1)
	}
	// Close the listener when the application closes.
	defer listener.Close()

	// Accept connections
	for {
		connFromClient, err := listener.Accept()
		if err != nil {
			fmt.Println("Error accepting connection:", err.Error())
			os.Exit(1)
		}
		// Set deadline for the connection
		// If the client does not send any data within 10 seconds, close the connection
		connFromClient.SetDeadline(time.Now().Add(IDLE_TIMEOUT))

		// Bind the connection to a server
		server := lb.bindConnection(connFromClient)
		fmt.Println("server:", server.this.RemoteAddr().String())
		
		// Handle connections in a new goroutine.
		go lb.handleClient(connFromClient, server)
	}
}