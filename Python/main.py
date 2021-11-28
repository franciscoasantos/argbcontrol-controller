import machine
import neopixel
import _thread
import time

from uwebsockets.protocol import Websocket

#Dados de conexão
wsHost = 'ws://192.168.1.239:3000/?clientId=0'

# [Pin, Qtd. Leds]
cama = [13, 107]
mesa1 = [12, 89]
mesa2 = [14, 70]

#Instância das Fitas de led
cama = neopixel.NeoPixel(machine.Pin(cama[0]), cama[1])
mesa1 = neopixel.NeoPixel(machine.Pin(mesa1[0]), mesa1[1])
mesa2 = neopixel.NeoPixel(machine.Pin(mesa2[0]), mesa2[1])

corAnterior = [0,0,0]
cor = [10,0,0]

def set_color(fita, cor):
  fita.fill(cor)
  fita.write()

def WebSocket():
    import uwebsockets.client
    try:
        websocket =  uwebsockets.client.connect(wsHost)
        while True:
            mensagem = websocket.recv()

            cor[0] = int(mensagem[1:4])
            cor[1] = int(mensagem[4:7])
            cor[2] = int(mensagem[7:10])
    except:
        print("erro")
        WebSocket()

# def fade():
#     # R = 0
#     # G = 1
#     # B = 2
#     while True:
#         if cor[0] > 0 & cor[2] == 0:
#             cor[0] = cor[0] - 1
#             cor[1] = cor[1] + 1
#             print(cor[1])
        
#         if cor[1] > 0 & cor[0] == 0:
#             cor[1] = cor[1] - 1         
#             cor[2] = cor[2] + 1         
        
#         if cor[2] > 0 & cor[1] == 0:
#             cor[0] = cor[0] + 1
#             cor[2] = cor[2] - 1

#         set_color(cama, cor)
#         time.sleep(0.05)

# fade()
#hello()

_thread.start_new_thread(WebSocket, ())

while True:
    if corAnterior != cor:
        corAnterior = [cor[0],cor[1],cor[2]]
        set_color(cama, cor)
        set_color(mesa1, cor)
        set_color(mesa2, cor)