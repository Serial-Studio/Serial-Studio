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
/// - Compile: `gcc -o udp_function_generator udp_function_generator.c -lm`
/// - Run: `./udp_function_generator [-p port] [-i interval] [-n num_functions]`
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
#include <unistd.h>
#include <arpa/inet.h>

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
  struct timespec ts;
  ts.tv_sec = (time_t)(milliseconds / 1000);
  ts.tv_nsec = (long)((milliseconds - (ts.tv_sec * 1000)) * 1e6);
  nanosleep(&ts, NULL);
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
  int udp_port = DEFAULT_UDP_PORT;
  int num_functions = DEFAULT_NUM_FUNCTIONS;
  double send_interval_ms = DEFAULT_SEND_INTERVAL_MS;

  // Parse command-line arguments
  int opt;
  int verbose = 0;
  while ((opt = getopt(argc, argv, "p:i:n:v")) != -1)
  {
    switch (opt)
    {
      case 'p':
        udp_port = atoi(optarg);
        break;
      case 'i':
        send_interval_ms = atof(optarg);
        break;
      case 'n':
        num_functions = atoi(optarg);
        if (num_functions < 1)
        {
          fprintf(stderr, "Number of functions must be at least 1\n");
          return 1;
        }
        break;
      case 'v':
        verbose = 1;
        break;
      default:
        fprintf(stderr,
                "Usage: %s [-p port] [-i interval] [-n num_functions]\n",
                argv[0]);
        return 1;
    }
  }

  print_tutorial();
  printf("Program started with the following options:\n");
  printf("- UDP Port:            %d\n", udp_port);
  printf("- Tx Interval (ms):    %.3f\n", send_interval_ms);
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
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
  {
    perror("Socket creation failed");
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
      perror("Send failed");
      close(sockfd);
      return 1;
    }

    // Optionally print the generated data
    if (verbose)
      printf("Sent data: %s", buffer);

    // Sleep for the specified interval
    sleep_ms(send_interval_ms);
  }

  close(sockfd);
  return 0;
}
