cd /workspaces/TJU-Socket-Lab/project

sudo docker build -t 15-441/641-project-1:latest -f ./DockerFile .

docker run -it -v /workspaces/TJU-Socket-Lab/project:/home/project-1/ --name Liso 15-441/641-project-1 /bin/bash
