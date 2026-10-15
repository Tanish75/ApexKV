#!/usr/bin/env bash
# ==============================================================================
# ApexKV - Automated AWS EC2 Deployment Script (Ubuntu 22.04 / 24.04 LTS)
# ==============================================================================
set -e

echo "=== [1/5] Updating packages and installing build tools ==="
sudo apt-get update -y
sudo apt-get install -y build-essential cmake g++ nginx git curl

echo "=== [2/5] Compiling ApexKV Engine & Test Suite ==="
chmod +x build.sh
./build.sh

echo "=== [3/5] Running Verification Tests ==="
./build/test_apexkv
./build/test_wal
./build/test_ttl
./build/test_thread_safety

echo "=== [4/5] Configuring Nginx to serve Interactive Web Demo on Port 80 ==="
sudo mkdir -p /var/www/apexkv
sudo cp index.html /var/www/apexkv/index.html

# Configure Nginx site
sudo bash -c 'cat > /etc/nginx/sites-available/apexkv <<EOF
server {
    listen 80 default_server;
    listen [::]:80 default_server;

    root /var/www/apexkv;
    index index.html;

    server_name _;

    location / {
        try_files \$uri \$uri/ =404;
    }
}
EOF'

# Enable site and restart Nginx
sudo rm -f /etc/nginx/sites-enabled/default
sudo ln -sf /etc/nginx/sites-available/apexkv /etc/nginx/sites-enabled/apexkv
sudo systemctl restart nginx
sudo systemctl enable nginx

echo "=== [5/5] Deployment Successful! ==="
PUBLIC_IP=$(curl -s http://169.254.169.254/latest/meta-data/public-ipv4 2>/dev/null || curl -s ifconfig.me || echo "<YOUR-EC2-PUBLIC-IP>")
echo ""
echo "=========================================================="
echo " ApexKV is successfully deployed on AWS EC2!"
echo "=========================================================="
echo " - Web Demo URL:     http://${PUBLIC_IP}"
echo " - Run C++ CLI:      ./build/apexkv"
echo " - Run Benchmarks:   ./build/benchmark_apexkv"
echo "=========================================================="
