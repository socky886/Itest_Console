#ifndef COMMON_H
#define COMMON_H
bool FileExists(const char *path);
bool find_file(const char *path);
void get_exe_path(char *buf);
int get_index();
bool get_Choose();
void get_input(char *buf);
bool demo_connect_tester(int *id = 0);
bool demo_Multi_connect_tester(int *id);
int print_power(int id);
int print_evm(int id);
int print_freq_err(int id);
int print_continuewave_power_and_freq_err(int id);
int print_bt_info(int id);
int print_zigbee_info(int id);

int print_power_2(int id, char *result);
int print_evm_2(int id, char *result);
int print_freq_err_2(int id, char *result);
int print_bt_info_2(int id, char *result);
int print_zigbee_info_2(int id, char *result);

int get_rf_vsg_config();
int get_rf_vsa_config();
int get_thread_config();
int get_connect_delta_config();

#define RF_CONFIG_FILE      "api_config.txt"
#define VSG_RF_PORT_TOKEN   "vsg_port"
#define VSA_RF_PORT_TOKEN   "vsa_port"
#define USER_COUNT_TOKEN    "users"
#define CONNECT_TIME_DELTA  "connect_delta"
#define WEIJUNFENG  "weijunfeng"

#define ENTER_LOCK(x)   while(InterlockedExchange(&x, 1) == 1) {Sleep(0);}
#define EXIT_LOCK(x)        InterlockedExchange(&x, 0)
#endif