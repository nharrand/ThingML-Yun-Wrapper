/*****************************************************
 *      THIS IS A GENERATED FILE. DO NOT EDIT.
 *        Implementation for Thing LinuxSerial
 *  Generated from ThingML (http://www.thingml.org)
 *****************************************************/

#include "LinuxSerial.h"

/*****************************************************************************
 * Implementation for type : LinuxSerial
 *****************************************************************************/

// Declaration of prototypes:
#ifdef __cplusplus
extern "C" {
#endif
void LinuxSerial_LinuxSerialImpl_OnExit(int state, struct LinuxSerial_Instance *_instance);
void LinuxSerial_send_serial_serial_rx(struct LinuxSerial_Instance *_instance, uint8_t b);
void LinuxSerial_send_serial_serial_closed(struct LinuxSerial_Instance *_instance);
void LinuxSerial_send_serial_serial_opened(struct LinuxSerial_Instance *_instance);
int16_t f_LinuxSerial_open_serial(struct LinuxSerial_Instance *_instance, char * device, int baudrate);
int16_t f_LinuxSerial_send_byte(struct LinuxSerial_Instance *_instance, int16_t device, uint8_t byte);
void f_LinuxSerial_start_receiver_process(struct LinuxSerial_Instance *_instance, int16_t device);
#ifdef __cplusplus
}
#endif

// Declaration of functions:
// Definition of function open_serial
int16_t f_LinuxSerial_open_serial(struct LinuxSerial_Instance *_instance, char * device, int baudrate) {
{
int result;
	
		printf("Opening ESUMS device at %s...\n", device);
		result = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	
		if (result < 0) {
			perror("Error opening Serial port");
		}
		else {
	
			//fcntl(result, F_SETFL, FNDELAY); // To make reading non-blocking ?
			fcntl(result, F_SETFL, 0); // To restore normal blocking read from the port
	
			printf("Configuring port %s...\n", device);
	
			struct termios port_settings;
	
			tcgetattr(result, &port_settings); // get current options
	
			cfsetispeed(&port_settings, B57600);    // set baud rates to 115200 ---------- Test with 57600
			cfsetospeed(&port_settings, B57600);
	
			port_settings.c_cflag &= ~PARENB;	// Set 8N1, No Parity
			port_settings.c_cflag &= ~CSTOPB;
			port_settings.c_cflag &= ~CSIZE;
			port_settings.c_cflag |= CS8;
	
			port_settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input mode
	
			port_settings.c_oflag &= ~OPOST; // Raw output mode
	
			tcsetattr(result, TCSANOW, &port_settings);    // apply the settings to the port
		    sleep(1); // wait a bit
        }
return result;
}
}

// Definition of function send_byte
int16_t f_LinuxSerial_send_byte(struct LinuxSerial_Instance *_instance, int16_t device, uint8_t byte) {
{
int n;
		unsigned char data[1];
		data[0] = byte;
        
		n = write(device, data, 1);
		sleep(1);// TOTEST ----------------------------------------------------------------------
		if (n < 0) {
			perror("Error writing to Serial device");
			return -1;
		}
		return 0;
}
}

// Fork a thread to execute the function f_LinuxSerial_start_receiver_process

// Struct for the parameters of the function  f_LinuxSerial_start_receiver_process
struct f_LinuxSerial_start_receiver_process_struct {
  struct LinuxSerial_Instance *_instance;
   int16_t device;

  pthread_mutex_t *lock;
  pthread_cond_t *cv;
};

// Definition of function start_receiver_process (executed in a separate thread)
void f_LinuxSerial_start_receiver_process_run(struct LinuxSerial_Instance *_instance, int16_t device)
{
  {
char buffer[INPUT_BUFFER_SIZE]; // Data read from the ESUSMS device

		int n; // used to store the results of select and read
		int i; // loop index
	
		while (1) {
	
			struct timeval timeout; // timeout waiting for incoming data
			timeout.tv_sec = TIMEOUT; 	// 10 second timeout
			timeout.tv_usec = 0;
	
			fd_set rdfs;		// The file descriptor to wait on
			FD_ZERO( &rdfs );
			FD_SET( device, &rdfs ); // set to the esusms fd
	
			n = select(device+1, &rdfs, NULL, NULL, &timeout); // Wait for timeout for something to be available to read
	
			if (n < 0) {
				perror("Error waiting for incoming data from Serial device");
				break;
			}
			else if (n == 0) { // timeout
				printf("Timeout waiting for incoming data from Serial device\n");
				break;
			}
			else { // there is something to read
				printf("[receiver] rx?\n");
				n = read(device, &buffer, INPUT_BUFFER_SIZE * sizeof(char));
	
				if (n<0) {
					perror("Error reading from Serial device");
					break;
				}
				else if (n==0) {
					printf("Nothing to read from Serial device\n"); // Should never happen unless there are too many transmission errors with wrong CRCs
					break;
				}
				else { // There are n incoming bytes in buffer
					printf("[receiver] rx! <%i>\n", n);
					char c;
					int size;
	
					for (i = 0; i<n; i++) {
						
						printf("[receiver] rx:<%i>\n", buffer[i]);
						
LinuxSerial_send_serial_serial_rx(_instance, buffer[i]);

					}
				}
			}
		}
LinuxSerial_send_serial_serial_closed(_instance);
}

}


void f_LinuxSerial_start_receiver_process_proc(void * p)
{
  // Get parameters from the main thread
  struct f_LinuxSerial_start_receiver_process_struct params = *((struct f_LinuxSerial_start_receiver_process_struct *) p);
  
  // Unblock the main thread
  pthread_mutex_lock(params.lock);
  pthread_cond_signal(params.cv);
  pthread_mutex_unlock(params.lock);
  
  // Run the function
  //f_LinuxSerial_start_receiver_process_run(params._instance, params.esums_device);
  f_LinuxSerial_start_receiver_process_run(params._instance, params.device);
}

// Operation called by the runtime and forking a new thread
void f_LinuxSerial_start_receiver_process(struct LinuxSerial_Instance *_instance, int16_t device)
{
  // Store parameters
  struct f_LinuxSerial_start_receiver_process_struct params;
  pthread_mutex_t lock;
  pthread_cond_t cv;
  params.lock = &lock;
  params.cv = &cv;
  params._instance = _instance;
  params.device = device;

  pthread_mutex_init(params.lock, NULL);
  pthread_cond_init(params.cv, NULL);
  //Start the new thread
  pthread_mutex_lock(params.lock);
  pthread_t thread; 
  pthread_create( &thread, NULL, f_LinuxSerial_start_receiver_process_proc, (void*) &params);
  // Wait until it has started and read its parameters
  pthread_cond_wait(params.cv, params.lock);
  // Realease mutex
  pthread_mutex_unlock(params.lock);
}

// On Entry Actions:
void LinuxSerial_LinuxSerialImpl_OnEntry(int state, struct LinuxSerial_Instance *_instance) {
switch(state) {
case LINUXSERIAL_LINUXSERIALIMPL_STATE:
_instance->LinuxSerial_LinuxSerialImpl_State = LINUXSERIAL_LINUXSERIALIMPL_RUNNING_STATE;
LinuxSerial_LinuxSerialImpl_OnEntry(_instance->LinuxSerial_LinuxSerialImpl_State, _instance);
break;
case LINUXSERIAL_LINUXSERIALIMPL_RUNNING_STATE:
break;
default: break;
}
}

// On Exit Actions:
void LinuxSerial_LinuxSerialImpl_OnExit(int state, struct LinuxSerial_Instance *_instance) {
switch(state) {
case LINUXSERIAL_LINUXSERIALIMPL_STATE:
LinuxSerial_LinuxSerialImpl_OnExit(_instance->LinuxSerial_LinuxSerialImpl_State, _instance);
break;
case LINUXSERIAL_LINUXSERIALIMPL_RUNNING_STATE:
break;
default: break;
}
}

// Event Handlers for incoming messages:
void LinuxSerial_handle_serial_serial_open(struct LinuxSerial_Instance *_instance, char * device, int baudrate) {
uint8_t LinuxSerial_LinuxSerialImpl_State_event_consumed = 0;
if (_instance->LinuxSerial_LinuxSerialImpl_State == LINUXSERIAL_LINUXSERIALIMPL_RUNNING_STATE) {
if (LinuxSerial_LinuxSerialImpl_State_event_consumed == 0 && 1) {
{
fprintf(stdout, "[LinuxSerial] received serial_open\n");

_instance->LinuxSerial_LinuxSerialImpl_serial_device__var = f_LinuxSerial_open_serial(_instance, device, baudrate);
if(_instance->LinuxSerial_LinuxSerialImpl_serial_device__var >  -1) {
fprintf(stdout, "[LinuxSerial] suceed to open\n");

f_LinuxSerial_start_receiver_process(_instance, _instance->LinuxSerial_LinuxSerialImpl_serial_device__var);
fprintf(stdout, "[LinuxSerial] started receiver process\n");

LinuxSerial_send_serial_serial_opened(_instance);
fprintf(stdout, "[LinuxSerial] opened\n");

}
}
LinuxSerial_LinuxSerialImpl_State_event_consumed = 1;
}
}
}
void LinuxSerial_handle_serial_serial_tx(struct LinuxSerial_Instance *_instance, uint8_t b) {
uint8_t LinuxSerial_LinuxSerialImpl_State_event_consumed = 0;
if (_instance->LinuxSerial_LinuxSerialImpl_State == LINUXSERIAL_LINUXSERIALIMPL_RUNNING_STATE) {
if (LinuxSerial_LinuxSerialImpl_State_event_consumed == 0 && 1) {
{
printf( "[LinuxSerial] wrote<%i>\n", b);
f_LinuxSerial_send_byte(_instance, _instance->LinuxSerial_LinuxSerialImpl_serial_device__var, b);
}
LinuxSerial_LinuxSerialImpl_State_event_consumed = 1;
}
}
}

// Observers for outgoing messages:
void (*LinuxSerial_send_serial_serial_rx_listener)(struct LinuxSerial_Instance*, uint8_t)= 0x0;
void register_LinuxSerial_send_serial_serial_rx_listener(void (*_listener)(struct LinuxSerial_Instance*, uint8_t)){
LinuxSerial_send_serial_serial_rx_listener = _listener;
}
void LinuxSerial_send_serial_serial_rx(struct LinuxSerial_Instance *_instance, uint8_t b){
if (LinuxSerial_send_serial_serial_rx_listener != 0x0) LinuxSerial_send_serial_serial_rx_listener(_instance, b);
}
void (*LinuxSerial_send_serial_serial_closed_listener)(struct LinuxSerial_Instance*)= 0x0;
void register_LinuxSerial_send_serial_serial_closed_listener(void (*_listener)(struct LinuxSerial_Instance*)){
LinuxSerial_send_serial_serial_closed_listener = _listener;
}
void LinuxSerial_send_serial_serial_closed(struct LinuxSerial_Instance *_instance){
if (LinuxSerial_send_serial_serial_closed_listener != 0x0) LinuxSerial_send_serial_serial_closed_listener(_instance);
}
void (*LinuxSerial_send_serial_serial_opened_listener)(struct LinuxSerial_Instance*)= 0x0;
void register_LinuxSerial_send_serial_serial_opened_listener(void (*_listener)(struct LinuxSerial_Instance*)){
LinuxSerial_send_serial_serial_opened_listener = _listener;
}
void LinuxSerial_send_serial_serial_opened(struct LinuxSerial_Instance *_instance){
if (LinuxSerial_send_serial_serial_opened_listener != 0x0) LinuxSerial_send_serial_serial_opened_listener(_instance);
}

