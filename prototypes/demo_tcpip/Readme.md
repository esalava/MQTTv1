# README #

Ejemplos que usan la interface sockets en Linux, basados en los ejemplos del capÃ­tulo de libro [Computer Systems: A Programmer's Perspective, 3/E](http://csapp.cs.cmu.edu/3e/home.html)

### CompilaciÃ³n ###

* Compilar el cliente: 
	* make client

* Compilar el servidor:
	* make server

* Compilar todo:
	* make
	
### Uso ###
Ejecutar el cliente:

```
./client <host> <port>
```
Ejemplo:

```
./client 127.0.0.1 8080
```

Ejecutar el cliente:

```
./server <port>
```
Ejemplo:

```
./server 8080
```