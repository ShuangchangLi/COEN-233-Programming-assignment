#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "Client.h"
#include "Tool.h"

enum Input
{
    Correct = 0,
    Wrong = 1
};

static int PAYLOAD_LENGTH = 5;

int main()
{
    // init the parameters
    int client_socket;
    int segment_no = 0;
    char buffer[1024];
    int input;

    struct Data_Packet data_packet;
    struct sockaddr_in server_address;

    while(1)
    {
        /*---- Show the instruction in console ----*/
        printf("Press 0 to send 5 correct packets\nPress 1 to send 1 correct packet and 4 wrong packets\nPress 2 to exit.\nYour input = ");

        
        /*---- Get the Inputs from console ----*/
        scanf("%d", &input);
        
        // Create client socket
        client_socket = socket(AF_INET, SOCK_DGRAM, 0);
        
        if (client_socket < 0)
            error("ERROR opening socket");
        
        // Set timer options on socket to 3 seconds
        struct timeval timeout = {3, 0};
        
        // init the socket optional settings
        // setsockopt if success return 0, if fail return -1
        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        {
            // handle the error
            error(" setsockopt Error !");
        }
        
        // Init server address
        server_address = GetServerAddress(PORT_NO);
        
        
        if (input == Correct)
        {
            for(int i = 0; i < 5; i = i + 1){
                data_packet = GenerateDataPacketToSend(segment_no, PAYLOAD_LENGTH, PAYLOAD_LENGTH, END_PACKET_ID);
                int packet_length = initPacketFromBuffer(data_packet, buffer);
                
                printf("-----------------------------------------------------\n");
                printf("Packet %d Sent: \n", segment_no);
                
                sendto(client_socket, buffer, packet_length, 0, (struct sockaddr *)&server_address, sizeof server_address);
                receiveHandler(client_socket, server_address, packet_length, buffer);
                
                segment_no++;
            }
        }
        else if (input == Wrong)
        {
            int i = 0;
            while(i < 5)
            {
                switch (i)
                {
                    case 0:
                        // right data packet
                        data_packet = GenerateDataPacketToSend(segment_no, PAYLOAD_LENGTH, PAYLOAD_LENGTH, END_PACKET_ID);
                        segment_no++;
                        break;
                    case 1:
                        // Out of sequence
                        data_packet = GenerateDataPacketToSend(segment_no + 100, PAYLOAD_LENGTH, PAYLOAD_LENGTH, END_PACKET_ID);
                        break;
                    case 2:
                        // Length mismatch
                        data_packet = GenerateDataPacketToSend(segment_no, 8, PAYLOAD_LENGTH, END_PACKET_ID);
                        break;
                    case 3:
                        // End of packet missing
                        data_packet = GenerateDataPacketToSend(segment_no, PAYLOAD_LENGTH, PAYLOAD_LENGTH, '\0');
                        break;
                    case 4:
                        // Duplicate packet
                        data_packet = GenerateDataPacketToSend(segment_no - 1, PAYLOAD_LENGTH, PAYLOAD_LENGTH, END_PACKET_ID);
                        break;
                }
                
                
                int packet_length = initPacketFromBuffer(data_packet, buffer);
                
                printf("-----------------------------------------------------\n");
                printf("Packet %d Sent: \n", segment_no);
                
                sendto(client_socket, buffer, packet_length, 0, (struct sockaddr *)&server_address, sizeof server_address);
                receiveHandler(client_socket, server_address, packet_length, buffer);
                
                
                i++;
            }
        }
        else if (input == 2)
        {
            break;
        }
    }
    
    return 0;
}
