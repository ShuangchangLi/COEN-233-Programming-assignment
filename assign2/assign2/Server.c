#include "Server.h"

int main()
{
    struct DataPacket data_packet;
    int num_subscribers = getNumberOfSubscribers();
    struct SubscriptionInfo subscriber_info[num_subscribers];
    readVerificationDatabaseFromTxt(subscriber_info);

    int server_socket;
    char buffer[1024];
    struct sockaddr_in server_address;

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    /*---- Configure server address struct ----*/
    server_address = getServerAddress(PORT_NO);

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    struct sockaddr sender;
    socklen_t sendsize = sizeof(sender);
    memset(&sender, 0, sizeof(sender));
    showDatabaseFromTxt();
    while (1)
    {
        int recv_len = recvfrom(server_socket, buffer, sizeof(buffer), 0, &sender, &sendsize);
        if (recv_len >= 0)
        {
            data_packet = parsePacketFromBuffer(buffer);
            printf("\n");
            printf("Received subscriber packet!\n");

            struct DataPacket
            {
                unsigned short start_packet_id;
                char client_id;
                unsigned short status;
                char segment_no;
                char length;
                struct Payload payload;
                unsigned short end_packet_id;
            };
            struct Payload
            {
                char technology;
                unsigned int subscriber_no;
            };
//            printf("%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\n",data_packet.start_packet_id,data_packet.client_id,data_packet.status,data_packet.segment_no,data_packet.length,data_packet.payload.subscriber_no,data_packet.payload.technology,data_packet.end_packet_id);
            data_packet.status = verify(num_subscribers, subscriber_info, data_packet);
//            printStatus(data_packet.status);
           //  short status=data_packet.status;
            switch (data_packet.status)
            {
                case ACCESS_OK:
                    printf("Responding with Subscriber status: 0xFFFB       Subscriber Access Granted");
                    break;
                case NOT_PAID:
                    printf("Responding with Subscriber status: 0xFFF9       Subscriber Not Paid");
                    break;
                case NOT_EXIST:
                    printf("Responding with Subscriber status: 0xFFFA       Subscriber Not Exist");
                    break;
            }
            int packet_length = generatePacketBufferToSend(data_packet, buffer);
            sendto(server_socket, buffer, packet_length, 0, (struct sockaddr *)&sender, sendsize);
            printf("\n");
        }
    }

    return 0;
}
