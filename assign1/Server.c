
#include <string.h>
#include "Tool.h"
#include "Server.h"
#include <stdbool.h>

int main()
{
    int server_socket;
    char buffer[1024];
    int current_segment_number = 0;
    int input;

    struct sockaddr_in server_addr;
    struct Data_Packet data_packet;
    struct Ack_Packet ack_packet;
    struct Reject_Packet reject_packet;

    /*---- Init the socket ----*/
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_socket < 0)
    {
        error("ERROR opening socket");
    }

    /*---- Initial server address ----*/
    server_addr = GetServerAddress(PORT_NO);

    /*---- Bind the address struct to the socket ----*/
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    struct sockaddr sender;
    socklen_t send_size = sizeof(sender);
    memset(&sender, 0, sizeof(sender));

    printf("Server Start\n");

    // Start to listen for packets from client
    while (1)
    {
        int packet_to_send_length = 0;

        recvfrom(server_socket, buffer, sizeof(buffer), 0, &sender, &send_size);

        bool ack = false;
        data_packet = generateDataPacketFromBuffer(buffer);
        


        if (data_packet.end_packet_id != END_PACKET_ID) //Check Error case - End packet missing (must check this at first)
        {
            reject_packet = generateRejectPacket(data_packet.segment_no, REJECT_END_OF_PACKET_MISSING);
            printf("Received Packet %d , Expecting Packet %d\n", data_packet.segment_no, current_segment_number);
            printf("Error: End of Packet Missing! REJECT CODE - %x\n", reject_packet.reject_sub_code);
        }
        else if (data_packet.segment_no == current_segment_number-1) // Check Error Case - Duplicate Packet
        {
            reject_packet = generateRejectPacket(data_packet.segment_no, REJECT_DUPLICATE_PACKET);
            printf("Received Packet %d , Expecting Packet %d\n", data_packet.segment_no, current_segment_number);
            printf("Error: Duplicate Packet! REJECT CODE - %x\n", reject_packet.reject_sub_code);
        }
        else if (data_packet.segment_no != current_segment_number) // Check Error case - Packet out of sequence
        {
            reject_packet = generateRejectPacket(data_packet.segment_no, REJECT_OUT_OF_SEQUENCE);
            printf("Received Packet %d , Expecting Packet %d\n", data_packet.segment_no, current_segment_number);
            printf("Error: Out of sequence! REJECT CODE - %x\n", reject_packet.reject_sub_code);
        }
        else if (data_packet.length != data_packet.payload.length) // Check Error Case - Length Mismatch
        {
            reject_packet = generateRejectPacket(data_packet.segment_no, REJECT_LENGTH_MISMATCH);
            printf("Received Packet %d , Expecting Packet %d\n", data_packet.segment_no, current_segment_number);
            printf("Error: Length Mismatch! REJECT CODE - %x\n", reject_packet.reject_sub_code);
        }
        else
        {
            ack = true;
            current_segment_number++;
            printf("Received Packet %d , Expecting Packet %d\n", data_packet.segment_no, current_segment_number);
            //Initialize ACK packet
            ack_packet = generateAckPacket(data_packet.segment_no);
            printf("Sending ACK\n");
        }

        packet_to_send_length = ack ? generateBufferFromAckPacket(ack_packet, buffer) : generateBufferFromRejectPacket(reject_packet, buffer);
        sendto(server_socket, buffer, packet_to_send_length, 0, (struct sockaddr *)&sender, send_size);

        printf("\n\n");
    }

    return 0;
}
