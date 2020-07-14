import sys
import struct

comandos = {'nop': 0x01, 
            'iadd': 0x02, 
            'isub': 0x05, 
            'iand': 0x08, 
            'ior': 0x0B,
            'dup': 0x0E,
            'pop': 0x10,
            'swap': 0x13,
            'bipush': 0x19,
            'iload': 0x1C, 
            'istore': 0x22, 
            'wide': 0x28, 
            'ldc_w': 0x32, 
            'iinc': 0x36, 
            'goto': 0x3C, 
            'iflt': 0x43,
            'ifeq': 0x47, 
            'if_icmpeq': 0x4B,
            'invokevirtual': 0x55,
            'ireturn': 0x6B }

def checar_sintaxe(programa):
  labels = []
  check_fim = True
  #Labels
  for line in programa:
    if len(line) > 1 and line[0] not in comandos:
      labels.append(line[0])

  for line in programa:

    if len(line) == 2:
      if line[0] == "bipush":
        if line[1].isnumeric() == True:
          check_fim = True
        else: return False
      elif line[0] == "iadd" or line[0] == "iand" or line[0] == "ior" or line[0] == "isub" or line[0] == "nop":
        return False
      elif line[0] == "if_icmpeq" or line[0] == "goto":
        if line[1] in labels:
          check_fim = True
        else: 
          return False
      elif line[0] == "iload" or line[0] == "istore":
        
        if line[1].isnumeric() == False:
          check_fim = True
        else: 
          return False
    if len(line) > 2:
      
      if line[1] == "bipush":
        if line[2].isnumeric() == True:
          check_fim = True
        else: 
          return False
      else: 
        check_fim = True
  return check_fim

#Função que escreve o programa no .exe
def escrita_prog(prog):
  # Converte o array de bytes em bytes
  bytes_final = bytes(prog)

  # Escreve o arquivo final
  with open("testedaDANI.exe", 'wb') as binary_output:
    #Escreve o texto ou bytes no arquivo
    bytes_escritos = binary_output.write(bytes_final)
  
def assembler(programa):
  #comandos que tem 2bytes de tamanho
  fixBytes = ['goto', 'if_icmpeq']

  labelsEsq = {} #Labels da esquerda {Main: posiçao, rep: posição,}
  labelsDir = {} #Labels da direita {Main: [valor], rep: [valor],}
  contador_byte = 0 #tamanho do programa

  lista_byte = []

  variaveis = [] 

  #Acha as Labels da esquerda e da direita
  for line in programa:
    if len(line) > 1 and line[0] not in comandos:
      labelsEsq[line[0]] = 0
    if len(line)> 1 and line[0] in fixBytes:
      labelsDir[line[1]] = []

  # print("Labels da Esquerda: ", labelsEsq)
  # print("Labels da Direita: ", labelsDir, "\n")
      
  #Cálculo do tamanho do programa, das lebals e variáveis.
  for line in programa: 
    #Primeiro caso: Linha nao tem label, apenas operador e operando
    if len(line) == 2 and line[0] not in labelsEsq and line[1] not in labelsDir:
      if not line[1].isnumeric() and line[1] not in variaveis: #Encontrar as variáveis
        variaveis.append(line[1])
      contador_byte += len(line)

    #Segundo caso: possui operador e Label
    elif len(line) == 2 and line[1] in labelsDir:
      labelsDir[line[1]].append(contador_byte + 1) #Diz o valor dessa label
      contador_byte += len(line) + 1
    elif len(line) == 2 and line[0] in labelsEsq:
      labelsEsq[line[0]] = contador_byte + 1 #Diz o valor dessa label
      contador_byte += len(line) - 1

    #Possui label operador e operando
    elif len(line) == 3 and line[0] in labelsEsq:
      labelsEsq[line[0]] = contador_byte + 1 #Diz o valor dessa label
      contador_byte += len(line) - 1 

    elif line[-1] not in labelsEsq and line[-1] not in variaveis and line[-1] not in comandos:
      variaveis.append(line[-1])
    else: 
      contador_byte += len(line) 

  #Variáveis não numéricas
  varNotnum = []
  for var in variaveis:
     if var.isnumeric() == False:
      varNotnum.append(var)

  # print("=-="*20)
  # print("Variáveis: ", variaveis)
  # print("Labels direita com valor:::::  ", labelsDir)
  # print("Labels esquerda com valor::::  ", labelsEsq)
  # print("=-="*20)
  Q_bytes = []
  #print("\nTAMANHO DO PROGRAMA (sem os 20 da inicialização): \033[0;37;42m %s \033[0m"% contador_byte)

  prog = bytearray() #ex.: bytearray(b'\x01\x02\x03\x04\x05')

  Q_bytes = struct.pack('<I', 20 + contador_byte)
  # print("Q : ", Q_bytes[0], "\n")

  for b in Q_bytes:
    prog.append(b)

  #INICIALIZADOR
  registradores = [
    0x7300, # INIT
    0x0006, # CPP
    0x1001, # LV 
    0x0400, # PC
    0x1001 + len(varNotnum) # SP
  ]
 
  for reg in registradores:
    # '<' Little-endian; 'I' unsigned int
    reg = struct.pack('<I', reg)
    for reg_byte in reg:
      prog.append(reg_byte)
  #Cálculo da distância entre as labels
  #onde as distâncias ficarão na LabelsDir, nesse formato:
  #{'fim': [8], 'rep': [-14]}
  contador = 1
  checkList = []
  for line in programa:
    if len(line) > 1 and line[0] not in comandos:
      contador -= 1
    if line[0] in fixBytes:
      if line[1] not in checkList:
        checkList.append(line[1])
        label = line[1]
        for i in range(0, len(labelsDir[line[1]])):
          labelsDir[label][i] = labelsEsq[label] - labelsDir[label][i]
      contador += 1
    for i in line:
      contador += 1

  #print("\nLabel com suas respectivas distâncias:::::  ", labelsDir)
  #Tradução do programa em si

  for line in programa:  
    for i in line:
      if i in comandos:
        prog.append(int(comandos[i]))
      elif i.isnumeric() == True:
        prog.append(int(i))
      elif i in varNotnum:
        prog.append(int(varNotnum.index(i)))
    if len(line) > 1:
      if line[1] in labelsDir:
        label = line[1]

        valorIni = hex(labelsDir[label][0] & (2**16-1)) #transforma o numero negativo em um hexa correspondente
        valorFim = int(valorIni, 16) #de hexa para inteiro
        valorConvertido = struct.pack('>I', valorFim) #Converte para "Big Endian"
        for b in range(2, len(valorConvertido)):
          prog.append(int(valorConvertido[b]))
        del labelsDir[label][0]
  #       print("LISTA DA LABEL DIREITA: ", valorConvertido)
      
  # print("\nPrograma em sí ==> ", prog)
  escrita_prog(prog)
      

def main():
  programa = []

  with open(sys.argv[1], 'r') as programa:
    programa = [line.split() for line in programa]
    #exemplo: [['Main', bipush', '21'], ['istore', 'x'],...]
    print("------------------------------------------------")
    
  print("RESULTADO DO CHECK: ", checar_sintaxe(programa))
  if checar_sintaxe(programa) == True:
    assembler(programa)
    print("=-=-=- Programa .exe gerado! =-=-=-")
  else:
    print("Tem algo errado com seu assembly :(")
  

main()
