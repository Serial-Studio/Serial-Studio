sudo docker build --no-cache -f Dockerfile_Ubuntu -t murmele/mdflib:ubuntu_22.04 .
sudo docker build --no-cache -f Dockerfile_Windows -t murmele/mdflib:windows .
sudo docker login
sudo docker push murmele/mdflib:ubuntu_22.04
