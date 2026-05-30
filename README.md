# Encoder App
## What is this app? ##
This app allows to encrypt and decrypt your text using such algorythms as RSA, Kuznechik and Stribog. 
The code represented in this repo is server-side app. It handles REST requests of adding users, logging in
your account and deleting your account, ecnryption and decryption of your text

## Main features ##
- Three access levels: admin, user and guest
- Encryption and decryption using RSA algorithm
- Encryption and decryption using Grasshopper algorithm
- Calculating hash code using Stribog alogrithm

## Installation guide ## 
1. Clone this repo: ```git clone https://github.com/Iskander919/encoder-server.git```
2. Create a Docker image of this app: go to the root directory of the project and execute\
```docker-compose build```
```docker-compose up```
3. Now you can send requests on localhost:8080