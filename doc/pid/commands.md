curl -v -X POST -F "pid_kp=0.2" -F "pid_ki=0" -F "pid_kd=0.1" -F "pid_iterm_min=0" http://192.168.125.123/api/config
curl -v -X POST -F "pid_kp=0.3" -F "pid_ki=0" -F "pid_kd=0.1" -F "pid_iterm_min=0" http://192.168.125.123/api/config
curl -v -X POST -F "pid_kp=0.3" -F "pid_ki=0" -F "pid_kd=0.2" -F "pid_iterm_min=0" http://192.168.125.123/api/config
curl -v -X POST -F "pid_kp=0.4" -F "pid_ki=0.01" -F "pid_kd=0.2" -F "pid_iterm_min=-5" http://192.168.125.123/api/config
curl -v -X POST -F "pid_kp=0.4" -F "pid_ki=0" -F "pid_kd=0.2" -F "pid_iterm_min=0" http://192.168.125.123/api/config
curl -v -X POST -F "pid_kp=0.6" -F "pid_ki=0.01" -F "pid_kd=0.8" -F "pid_iterm_min=-10" http://192.168.125.123/api/config
curl -v -X POST -F "pid_kp=0.6" -F "pid_ki=0" -F "pid_kd=0.8" -F "pid_iterm_min=0" http://192.168.125.123/api/config
