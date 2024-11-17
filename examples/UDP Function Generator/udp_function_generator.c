///
/// @file udp_function_generator.c
/// @author Alex Spataru
/// @brief A UDP-based waveform generator for sine, triangular, sawtooth, and
///      square waves.
///
/// This program generates waveform data and sends it via UDP to a specified
/// port. Users can configure the number of functions, their forms, frequencies,
/// and phases.
///
/// Features:
/// - Generate multiple waveforms of different types: sine, triangular,
///   sawtooth, and square.
/// - Configurable send interval and UDP port.
/// - Option to print generated data to the console.
/// - Frequency validation to warn about potential aliasing.
///
/// Usage:
/// - Compile: (UNIX): gcc -o udp_function_generator udp_function_generator.c
/// -lm
/// - Compile (Windows):  gcc -o udp_function_generator.exe
/// udp_function_generator.c -lws2_32 -lm
/// - Run: ./udp_function_generator [-p port] [-i interval] [-n num_functions]
///
/// Options:
/// - `-p <port>`: UDP port (default: 9000)
/// - `-i <interval>`: Send interval in milliseconds (default: 1.0 ms)
/// - `-n <num_functions>`: Number of waveforms (default: 1)
/// - `-v`: Enable verbose output
///
/// Example: `./udp_function_generator -p 9000 -i 5 -n 3 -v`
///
/// Press Ctrl+C to terminate the program.
///

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <winsock2.h>
#  include <windows.h>
#  pragma comment(lib, "ws2_32.lib")
#else
#  include <time.h>
#  include <unistd.h>
#  include <arpa/inet.h>
#endif

#define MAX_BUFFER_SIZE 128
#define TWO_PI 6.28318530718

#define DEFAULT_UDP_PORT 9000
#define DEFAULT_NUM_FUNCTIONS 1
#define DEFAULT_SEND_INTERVAL_MS 1.0

/**
 * @brief Sleep for a specified number of milliseconds.
 *
 * This function uses `nanosleep` to pause the program for the specified
 * duration in milliseconds, including fractional values.
 *
 * @param milliseconds Time in milliseconds to sleep.
 */
void sleep_ms(double milliseconds)
{
#ifdef _WIN32
  Sleep((DWORD)(milliseconds));
#else
  struct timespec ts;
  ts.tv_sec = (time_t)(milliseconds / 1000);
  ts.tv_nsec = (long)((milliseconds - (ts.tv_sec * 1000)) * 1e6);
  nanosleep(&ts, NULL);
#endif
}

/**
 * @brief Validate the waveform type.
 *
 * Checks whether the provided waveform type is one of the supported
 * types: "sine", "triangle", "saw", or "square".
 *
 * @param wave_type The type of waveform to validate.
 * @return 1 if the waveform type is valid, 0 otherwise.
 */
int validate_wave_type(const char *wave_type)
{
  return (strcmp(wave_type, "sine") == 0 || strcmp(wave_type, "triangle") == 0
          || strcmp(wave_type, "saw") == 0 || strcmp(wave_type, "square") == 0);
}

/**
 * @brief Calculate waveform values based on type, frequency, and phase.
 *
 * Generates a value for the specified waveform type at the given
 * frequency and phase. The output value is scaled to a 0-5V range.
 *
 * @param wave_type The type of waveform (sine, triangle, sawtooth, square).
 * @param frequency Frequency of the waveform (not used directly here).
 * @param phase Current phase of the waveform in radians.
 * @return Generated waveform value in the 0-5V range.
 */
float generate_wave_value(const char *wave_type, float frequency, float phase)
{
  if (strcmp(wave_type, "sine") == 0)
    return (sinf(phase) + 1.0) * 2.5;

  else if (strcmp(wave_type, "triangle") == 0)
    return fabsf(fmodf(phase / TWO_PI, 1.0f) * 2.0f - 1.0f) * 5.0f;

  else if (strcmp(wave_type, "saw") == 0)
    return fmodf(phase / TWO_PI, 1.0f) * 5.0f;

  else if (strcmp(wave_type, "square") == 0)
    return (sinf(phase) >= 0 ? 5.0f : 0.0f);

  return 0.0f;
}

/**
 * @brief Print a quick tutorial on program usage.
 *
 * This function explains how to use the program, including available options,
 * and gives an example of a typical command.
 */
void print_tutorial()
{
  // clang-format off
  printf("UDP Function Generator Tutorial:\n");
  printf("- Generate multiple waveforms (sine, triangle, saw, square).\n");
  printf("- Specify number of functions, waveform type, frequency, and phase.\n");
  printf("- Options:\n");
  printf("  -p <port>          : UDP port (default: 9000)\n");
  printf("  -i <interval>      : Send interval in milliseconds (default: 1.0 ms)\n");
  printf("  -n <num_functions> : Number of waveforms to generate (default: 1)\n");
  printf("  -v                 : Enable verbose output\n\n");
  printf("Example: ./udp_function_generator -p 9000 -i 5 -n 3 -v\n\n");
  // clang-format on
}

/**
 * @brief Validate frequencies for aliasing issues and warn the user.
 *
 * This function checks whether the specified frequency is too high for
 * the given sampling interval. If the frequency exceeds the Nyquist rate
 * or approaches it too closely (e.g., > 80%), it warns the user about
 * potential waveform distortion.
 *
 * @param frequency Frequency of the waveform in Hz.
 * @param send_interval_ms Time between data points in milliseconds.
 */
void validate_frequency(float frequency, double send_interval_ms)
{
  double nyquist_rate = 1.0 / (2.0 * (send_interval_ms / 1000.0));
  double safe_rate = 0.8 * nyquist_rate;

  if (frequency >= nyquist_rate)
  {
    printf("Warning: Frequency %.2f Hz equals or exceeds the Nyquist rate "
           "(%.2f Hz). "
           "Waveform will be severely distorted.\n",
           frequency, nyquist_rate);
  }

  else if (frequency > safe_rate)
  {
    printf("Warning: Frequency %.2f Hz approaches the Nyquist rate (%.2f Hz). "
           "Consider reducing it below %.2f Hz to ensure smooth waveform "
           "reconstruction.\n",
           frequency, nyquist_rate, safe_rate);
  }
}

/**
 * @brief Parses command-line arguments and configures program settings.
 *
 * This function processes command-line arguments passed to the program
 * and updates the provided variables for UDP port, send interval, number
 * of waveforms, and verbose mode based on the user's input. If invalid or
 * incomplete arguments are detected, the function prints an error message
 * and terminates the program.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line argument strings.
 * @param udp_port Pointer to an integer where the UDP port will be stored.
 * @param send_interval_ms Pointer to a double where the send interval in
 *                         milliseconds will be stored.
 * @param num_functions Pointer to an integer where the number of waveforms
 *                      will be stored.
 * @param verbose Pointer to an integer where the verbose flag will be stored
 *                (1 for enabled, 0 for disabled).
 */
void parse_arguments(int argc, char *argv[], int *udp_port,
                     double *send_interval_ms, int *num_functions, int *verbose)
{
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
      *udp_port = atoi(argv[++i]);

    else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
      *send_interval_ms = atof(argv[++i]);

    else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc)
    {
      *num_functions = atoi(argv[++i]);
      if (*num_functions < 1)
      {
        fprintf(stderr, "Number of functions must be at least 1\n");
        exit(1);
      }
    }

    else if (strcmp(argv[i], "-v") == 0)
      *verbose = 1;

    else
    {
      fprintf(stderr,
              "Usage: %s [-p port] [-i interval] [-n num_functions] [-v]\n",
              argv[0]);
      exit(1);
    }
  }
}

/**
 * @brief Main program entry point.
 *
 * This function parses command-line arguments, collects user input for
 * waveform details, validates the inputs, and generates waveforms to
 * send via UDP.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Exit status of the program.
 */
int main(int argc, char *argv[])
{
#ifdef _WIN32
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
  {
    fprintf(stderr, "WSAStartup failed\n");
    return 1;
  }
#endif

  // Parse command-line arguments
  double send_interval_ms = DEFAULT_SEND_INTERVAL_MS;
  int udp_port = DEFAULT_UDP_PORT, num_functions = DEFAULT_NUM_FUNCTIONS,
      verbose = 0;
  parse_arguments(argc, argv, &udp_port, &send_interval_ms, &num_functions,
                  &verbose);

  print_tutorial();
  printf("Program started with the following options:\n");
  printf("- UDP Port:            %d\n", udp_port);
  printf("- Tx Interval (ms):    %.6f\n", send_interval_ms);
  printf("- Number of waveforms: %d\n\n", num_functions);

  // Collect waveform details from the user
  char wave_types[num_functions][16];
  float frequencies[num_functions];
  float phases[num_functions];
  for (int i = 0; i < num_functions; i++)
  {
    while (1)
    {
      printf("Enter details for waveform %d:\n", i + 1);
      printf("- Wave type (sine, triangle, saw, square): ");
      scanf("%s", wave_types[i]);
      if (validate_wave_type(wave_types[i]))
        break;

      else
      {
        printf("Error: Invalid waveform type. Please enter 'sine', 'triangle', "
               "'saw', or 'square'.\n");
      }
    }

    printf("- Frequency (Hz): ");
    scanf("%f", &frequencies[i]);
    printf("- Phase (radians): ");
    scanf("%f", &phases[i]);
    validate_frequency(frequencies[i], send_interval_ms);
    printf("\n");
  }

  // Create UDP socket
#ifdef _WIN32
  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
#endif
  if (sockfd < 0)
  {
    perror("Socket creation failed");
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  // Set up destination address
  struct sockaddr_in dest_addr;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(udp_port);
  dest_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  // Calculate phase increments for each waveform
  double phase_increment[num_functions];
  for (int i = 0; i < num_functions; i++)
    phase_increment[i] = TWO_PI * frequencies[i] * (send_interval_ms / 1000.0);

  // Print run status
  printf("The program is now generating real-time functions...\n\n");
  printf("To visualize the data in Serial Studio:\n");
  printf("  1. Set the I/O interface to \"Network Socket\".\n");
  printf("  2. Enable \"Quick Plot\" operation mode.\n");
  printf("  3. Select \"UDP\" as the Socket Type.\n");
  printf("  4. Set the Host to \"localhost\".\n");
  printf("  5. Configure both the Local and Remote ports to %d.\n\n", udp_port);
  printf("Enjoy your testing experience! :)\n\n");

  // Main loop: Generate and send waveforms
  while (1)
  {
    float values[num_functions];
    for (int i = 0; i < num_functions; i++)
    {
      values[i] = generate_wave_value(wave_types[i], frequencies[i], phases[i]);
      phases[i] += phase_increment[i];
      if (phases[i] > TWO_PI)
        phases[i] -= TWO_PI;
    }

    // Format data as a comma-separated string
    char buffer[MAX_BUFFER_SIZE];
    int offset = 0;
    for (int i = 0; i < num_functions; i++)
    {
      offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%.2f",
                         values[i]);

      if (i < num_functions - 1)
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",");
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

    // Send data via UDP
    // clang-format off
    ssize_t sent_bytes = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    // clang-format on
    if (sent_bytes < 0)
    {
#ifdef _WIN32
      fprintf(stderr, "Send failed: %d\n", WSAGetLastError());
      closesocket(sockfd);
      WSACleanup();
#else
      perror("Send failed");
      close(sockfd);
#endif
    }

    // Optionally print the generated data
    if (verbose)
      printf("Sent data: %s", buffer);

    // Sleep for the specified interval
    sleep_ms(send_interval_ms);
  }

#ifdef _WIN32
  closesocket(sockfd);
  WSACleanup();
#else
  close(sockfd);
#endif
  return 0;
}
