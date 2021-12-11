from machine import Pin
import neopixel
import _thread
import time

#Dados de conexão
wsHost = 'ws://services.franciscosantos.net:3000/?clientId=0'

# [Pin, Qtd. Leds]
cama = [13, 107]
mesa1 = [12, 89]
mesa2 = [14, 70]

#Instância das Fitas de led
cama = neopixel.NeoPixel(Pin(cama[0], Pin.OUT), cama[1])
mesa1 = neopixel.NeoPixel(Pin(mesa1[0], Pin.OUT), mesa1[1])
mesa2 = neopixel.NeoPixel(Pin(mesa2[0], Pin.OUT), mesa2[1])

#0 = Estático | 1 = Fade
modo = [1]
corAnterior = [0,0,0]
cor = [255,0,0]

def set_color(fita, cor):
  fita.fill(cor)
  fita.write()

def set_color_all(cor): 
    cama.fill(cor)
    mesa1.fill(cor)
    mesa2.fill(cor)
    cama.write()
    mesa1.write()
    mesa2.write()


def WebSocket():
    import uwebsockets.client
    try:
        websocket =  uwebsockets.client.connect(wsHost)
        while True:
            mensagem = websocket.recv()
            modo[0] = int(mensagem[0:1])
            
            cor[0] = int(mensagem[1:4])
            cor[1] = int(mensagem[4:7])
            cor[2] = int(mensagem[7:10])
    except:
        print("Erro no servidor, reconetando...")
        time.sleep(3)
        WebSocket()

def Fade():
    r = int(cor[0])
    g = int(cor[1])
    b = int(cor[2])

    while True:
        if modo[0] == 1:
            if r > 0 and b == 0:
                r = r - 3
                g = g + 3
            
            if g > 0 and r == 0:
                g = g - 3        
                b = b + 3        
            
            if b > 0 and g == 0:
                r = r + 3
                b = b - 3

            set_color_all([r,g,b])
            time.sleep(0.1)
        else:
            break

_thread.start_new_thread(WebSocket, ())

while True:
    if modo[0] == 0:
        if corAnterior != cor:
            corAnterior = [cor[0],cor[1],cor[2]]
            set_color(cama, cor)
            set_color(mesa1, cor)
            set_color(mesa2, cor)
    else:
        Fade()