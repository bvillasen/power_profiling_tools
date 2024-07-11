#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream> 
#include <chrono>
#include <vector>
#include <unistd.h>

#include "rocm_smi/rocm_smi.h"

int64_t get_microseconds_stamp(std::chrono::high_resolution_clock::time_point &currentTime){
  currentTime = std::chrono::high_resolution_clock::now();
  auto timestamp = std::chrono::time_point_cast<std::chrono::microseconds>(currentTime);
  return timestamp.time_since_epoch().count(); 
}

std::string getCurrentTimestamp( int64_t microseconds, std::chrono::high_resolution_clock::time_point currentTime){
    char buffer[30];
    std::time_t tt;
    tt = std::chrono::system_clock::to_time_t ( currentTime );
    auto timeinfo = localtime (&tt);
    strftime (buffer,80,"%F %H:%M:%S",timeinfo);
    sprintf(buffer, "%s.%06ld",buffer,microseconds%1000000);
    return std::string(buffer);
}


void get_cray_pm_power(int dev_indx, int &power, int64_t &time_stamp ) {

    std::string cmd = "cat /sys/cray/pm_counters/accel";
    cmd.append( std::to_string(dev_indx) ) ;
    cmd.append( "_power" );

    std::string result;
    char buffer[128];
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != nullptr) {
                result += buffer;
            }
        }
        pclose(pipe);
    }
   std::stringstream ss(result);
    std::string token, s_power, s_timestamp;
    char delimiter = ' ';
    int indx = 0;
    while (std::getline(ss, token, delimiter)) {
        if ( indx == 0 ) s_power = token;
        if ( indx == 2 ) s_timestamp = token;
        indx += 1; 
    }
    power = stoi(s_power);
    time_stamp = stoll(s_timestamp);
    // std::cout << "power: " << power << " timestamp: " << time_stamp << std::endl;
}

int main(int argc, char* argv[]) {
  
  std::string output_file_name = argv[1];
  double time = std::stod(argv[2]);
  double sampling_freq = std::stod(argv[3]); //Hz
  // if ( argc >= 4 ) std::vector devices<int> = parse_devices_argument( argv[3] );
  // std::vector<int> devices = { 0 };
  // std::vector<int> devices = { 0, 1 };
  std::vector<int> devices = { 0, 1, 2, 3 };

  int gcds_per_gpu = 2;

  bool get_cray_counters = false;
  
  int n_gpus = devices.size();
  int n_samples = static_cast<int>( time * sampling_freq );
  uint64_t sampling_period = static_cast<uint64_t>( 1.0/sampling_freq * 1e6 ); //microsecs
  
  // std::cout << "Power Measure." << std::endl; 
  // std::cout << "Output file: " << output_file_name << std::endl;
  // std::cout << "N samples: " << n_samples << std::endl; 
  // std::cout << "Sampling time: " << time <<  " seconds." << std::endl; 
  // std::cout << "Sampling period: " << sampling_period/1000 <<  " milliseconds." << std::endl; 

  std::ofstream out_file(output_file_name);
  if (!out_file.is_open()) {
    std::cerr << "Error opening the output file: " << output_file_name << std::endl;
    return 1;
  }
  
  rsmi_status_t status, rsmi_status;
  uint32_t n_devices;
  status = rsmi_init(0);
  status = rsmi_num_monitor_devices(&n_devices);

  std::vector<std::string> times(n_samples);
  std::vector<std::vector<int>> rsmi_power_vals(n_samples, std::vector<int>(n_gpus, 0));
  std::vector<std::vector<int>> cray_power_vals(n_samples, std::vector<int>(n_gpus, 0));


  RSMI_POWER_TYPE rsmi_power_type;
  uint64_t rsmi_power;
  
  rsmi_status = rsmi_dev_power_get( 0, &rsmi_power, &rsmi_power_type);
  if ( rsmi_status == RSMI_STATUS_SUCCESS )            std::cout << "rocm-smi: dev_power_get success" << std::endl;
  else if ( rsmi_status == RSMI_STATUS_NOT_SUPPORTED ) std::cout << "rocm-smi: dev_power_get not supported" << std::endl;
  else if ( rsmi_status == RSMI_STATUS_INVALID_ARGS )  std::cout << "rocm-smi: dev_power_get invalid arguments" << std::endl;
  if (rsmi_power_type == RSMI_AVERAGE_POWER)           std::cout << "rocm-smi: dev_power_get returns the average power." << std::endl;
  else if ( rsmi_power_type == RSMI_CURRENT_POWER )    std::cout << "rocm-smi: dev_power_get returns the instantaneous power." << std::endl;
  else{
    std::cout << "ERROR: Invalid output from rocm-smi dev_power_get exiting." << std::endl;
    return -1;
  }

  // rsmi_statuss = rsmi_dev_power_ave_get (0, 0, &rsmi_power_avrg);
  // if ( rsmi_statuss == RSMI_STATUS_SUCCESS ){
  //   std::cout << "rocm-smi: power_ave_get success" << std::endl;
  //   get_rsmi_average_power[dev_indx] = true;
  // } 
  // else if ( rsmi_statuss == RSMI_STATUS_NOT_SUPPORTED ) std::cout << "rocm-smi: power_ave_get not supported" << std::endl;
  // else if ( rsmi_statuss == RSMI_STATUS_INVALID_ARGS ) std::cout << "rocm-smi: power_ave_get invalid arguments" << std::endl;

  // rsmi_statuss = rsmi_dev_current_socket_power_get( 0, &rsmi_power_socket );
  // if ( rsmi_statuss == RSMI_STATUS_SUCCESS ){
  //   std::cout << "rocm-smi: current_socket_power_get success" << std::endl;
  //   get_rsmi_socket_power[dev_indx] = true;
  // } 
  // else if ( rsmi_statuss == RSMI_STATUS_NOT_SUPPORTED ) std::cout << "rocm-smi: current_socket_power_get not supported" << std::endl;
  // else if ( rsmi_statuss == RSMI_STATUS_INVALID_ARGS ) std::cout << "rocm-smi: current_socket_power_get invalid arguments" << std::endl;

  // }


  std::cout << "Writing measurements to output file: " << output_file_name << std::endl;

  std::stringstream header;
  header << "#date time";
  for ( int dev_indx=0; dev_indx<n_gpus; dev_indx++ ){
   if (rsmi_power_type == RSMI_AVERAGE_POWER) header << " dev" + std::to_string(dev_indx) + "-rsmi_pow_average";
   if ( rsmi_power_type == RSMI_CURRENT_POWER )  header << " dev" + std::to_string(dev_indx) + "-rsmi_pow_socket";
  }
  if (get_cray_counters) {
    for ( int dev_indx=0; dev_indx<n_gpus; dev_indx++ ){
      header << " dev" + std::to_string(dev_indx) + "-cray_pm_counter"; 
    }
  }
  
  header << std::endl;
  out_file << header.str();

  
  int cray_power;
  int64_t cray_timestamp;
  int dev_id;

  std::chrono::high_resolution_clock::time_point current_time;
  int64_t microseconds, elapsed, remaining;

  microseconds = get_microseconds_stamp(current_time);
  // std::cout << "Starting power profiling at " << getCurrentTimestamp(microseconds, current_time) << std::endl;
  
  for ( int sample_indx=0; sample_indx < n_samples; sample_indx++ ){
    microseconds = get_microseconds_stamp(current_time);
    times[sample_indx] = getCurrentTimestamp(microseconds, current_time);

    for (int dev_indx=0; dev_indx < n_gpus; dev_indx++) {
      dev_id = devices[dev_indx];
      
      if (get_cray_counters){
        get_cray_pm_power( dev_id, cray_power, cray_timestamp );
        cray_power_vals[sample_indx][dev_indx] = cray_power;
      }

      rsmi_status = rsmi_dev_power_get( gcds_per_gpu*dev_indx, &rsmi_power, &rsmi_power_type);
      rsmi_power_vals[sample_indx][dev_indx] = rsmi_power/1000000;
    }

    // std::cout << "\nTime: " << times[sample_indx] << " microsecs: " << microseconds << " Power [W]: ";
    // for (int dev_indx=0; dev_indx<n_gpus; dev_indx++ ) std::cout << " " << rsmi_power_vals[sample_indx][dev_indx];
    // std::cout << std::endl;

    out_file << times[sample_indx] << " ";
    for ( int dev_indx=0; dev_indx<n_gpus; dev_indx++ ){
      out_file << rsmi_power_vals[sample_indx][dev_indx] << " ";
    }
    
    if (get_cray_counters){
      for ( int dev_indx=0; dev_indx<n_gpus; dev_indx++ ){
       out_file << cray_power_vals[sample_indx][dev_indx] << " ";
      }
    }
    
    out_file << std::endl;

    elapsed = get_microseconds_stamp(current_time) - microseconds;
    // std::cout << "Elapsed: " << elapsed << std::endl;
    remaining = sampling_period - elapsed;
    if (remaining > 0) usleep( remaining );
  }

  // microseconds = get_microseconds_stamp(current_time);
  // std::cout << "Finished power profiling at " << getCurrentTimestamp(microseconds, current_time) << std::endl;
  status = rsmi_shut_down();
  return 0;
}