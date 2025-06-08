Set the CPU governor to “performance” (once in your shell):
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

sudo update-initramfs -u
