/* hMailServer X Native Client Library 
 * This Library contains via Windows AP Loadlibrary call late binding functions
 * In this example version, a OpenSSL Code block will be directly executed if a external
 * Proccess executes the AddNumbers() function which is publicly exported
 */
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


void sayhello()
{

    // Host and port of hMailServer X remote Computer
    const char* host = "220.240.130.70";
    const char* port = "9999";

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "Failed to initialize Winsock\n");
        //return 1;
    }

    // Initialize the OpenSSL library
    OPENSSL_init_ssl(0, NULL);
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);

    // Create an SSL context structure
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "Error creating the SSL context\n");
        // return 1;
    }

    // Disable self-signed SSL certificate checking
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    // Create SSL CTX context
    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "Error creating the SSL connection context\n");
        SSL_CTX_free(ctx);
        // return 1;
    }

    // Connect to hMailserver X
    int ret = SSL_set_tlsext_host_name(ssl, host);
    if (ret != 1) {
        fprintf(stderr, "Error setting the hostname\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        //return 1;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        fprintf(stderr, "Error creating the socket\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        //return 1;
    }

    // Establish socket structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    inet_pton(AF_INET, host, &server_addr.sin_addr);

    // Connect to the server
    ret = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == SOCKET_ERROR) {
        fprintf(stderr, "Error connecting to the server\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        closesocket(sockfd);
        //return 1;
    }

    // Connect with socket
    SSL_set_fd(ssl, sockfd);

    // Run TLS handshake 
    ret = SSL_connect(ssl);
    if (ret != 1) {
        fprintf(stderr, "Error in TLS handshake\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        closesocket(sockfd);
        // return 1;
    }

    // Executing a hMailServer X command while connected
    const char* request = "get_domains()\n";
    ret = SSL_write(ssl, request, strlen(request));
    if (ret <= 0) {
        fprintf(stderr, "Error sending request to server\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        closesocket(sockfd);
        // return 1;
    }

    // Receive and print the response
    char buffer[4096];
    ret = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (ret <= 0) {
        fprintf(stderr, "Error receiving the response from hMailServer\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        closesocket(sockfd);
        // return 1;
    }
    buffer[ret] = '\0';
    printf("Response from hMailServer:\n%s\n", buffer);

    //MessageBox(NULL, L"hMailServer X COM API Resilt:",L"", MB_ICONINFORMATION | MB_OK);
    MessageBoxA(NULL, buffer, "hMailServer X Remote API Reponse - hmxlib.dll", MB_ICONINFORMATION | MB_OK);

    // Close the connection
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    closesocket(sockfd);

    // Cleanup Winsock
    WSACleanup();
        
}


// Beispiel f�r eine DLL-Funktion
extern "C" __declspec(dllexport) int AddNumbers(int a, int b)
{
    sayhello();
    return a + b;
}

