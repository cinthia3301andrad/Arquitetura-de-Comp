#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

using namespace std;

//definiÃ§es de tipos
typedef unsigned char byte;
typedef unsigned int  word;
typedef unsigned long microcode;

//estrutura para guardar uma microinstruÃ§ao decodificada
struct decoded_microcode
{
    word nadd;
    byte jam;
    byte sft;
    byte alu;
    word reg_w;
    byte mem;
    byte reg_r;
};

//FunÃ§Ãµes utilitÃ¡rias ======================
void write_microcode(microcode w) //Dado uma microinstrucao, exibe na tela devidamente espaÃ§ado pelas suas partes.
{
   unsigned int v[36];
   for(int i = 35; i >= 0; i--)
   {
       v[i] = (w & 1);
       w = w >> 1;
   }

   for(int i = 0; i < 36; i++)
   {
       cout << v[i];
       if(i == 8 || i == 11 || i == 13 || i == 19 || i == 28 || i == 31) cout << " ";
   }
}

void write_word(word w) //Dada uma palavra (valor de 32 bits / 4 bytes), exibe o valor binÃ¡rio correspondente.
{
   unsigned int v[32];
   for(int i = 31; i >= 0; i--)
   {
       v[i] = (w & 1);
       w = w >> 1;
   }

   for(int i = 0; i < 32; i++)
       cout << v[i];
}

void write_byte(byte b) //Dado um byte (valor de 8 bits), exibe o valor binÃ¡rio correspondente na tela.
{
   unsigned int v[8];
   for(int i = 7; i >= 0; i--)
   {
       v[i] = (b & 1);
       b = b >> 1;
   }

   for(int i = 0; i < 8; i++)
       cout << v[i];
}

void write_dec(word d) //Dada uma palavra (valor de 32 bits / 4 bytes), exibe o valor decimal correspondente.
{
   cout << (int)d << endl;
}
//=========================================

//sinalizador para desligar mÃ¡quina
bool halt = false;

//memoria principal
#define MEM_SIZE 0xFFFF+1 //0xFFFF + 0x1; // 64 KBytes = 64 x 1024 Bytes = 65536 (0xFFFF+1) x 1 Byte;
byte memory[MEM_SIZE]; //0x0000 a 0xFFFF (0 a 65535)

//registradores
word mar=0, mdr=0, pc=0, sp=0, lv=0, cpp=0, tos=0, opc=0, h=0;
byte mbr=0;

//barramentos
word bus_a=0, bus_b=0, bus_c=0, alu_out=0;

//estado da ALU para salto condicional
byte n=0, z=1;

//registradores de microprograma
word mpc;

//memÃ³ria de microprograma: 512 x 64 bits = 512 x 8 bytes = 4096 bytes = 4 KBytes.
//Cada microinstruÃ§Ã£o Ã© armazenada em 8 bytes (64 bits), mas apenas os 4,5 bytes (36 bits) de ordem mais baixa sÃ£o de fato decodificados.
//Os 28 bits restantes em cada posiÃ§Ã£o da memÃ³ria sÃ£o ignorados, mas podem ser utilizados para futuras melhorias nas microinstruÃ§Ãµes para controlar microarquiteturas mais complexas.
microcode microprog[512];

//carrega microprograma
void load_microprog()
{
        FILE *mp = fopen("microprog.rom", "rb");
    if (mp != NULL)
    {
        fread(microprog, sizeof(microcode), 512, mp);
        fclose(mp);
    }
    else
    {
        std::cout << "Não achamos o microprograma " << std::endl;
        exit(-1);
    }

}
//carrega programa na memÃ³ria principal para ser executado pelo emulador.
//programa escrito em linguagem de mÃ¡quina (binÃ¡rio) direto na memÃ³ria principal (array memory declarado mais acima).
void load_prog()
{
    FILE *f = fopen("testedaDANI.exe", "rb");

    word Q;

    fread(&Q, sizeof(word), 1, f); //os 4 bytes do cabeçalho

    fread(&memory[0], sizeof(byte), 20, f);

    word pc;

    memcpy(&pc, memory + 12, 4);

    fread(memory +pc +1, sizeof(byte), Q-20, f);

    fclose;
}



//exibe estado da mÃ¡quina
void debug(bool clr = true)
{
    if(clr) system("clear");

    cout << "MicroinstruÃ§Ã£o: ";
    write_microcode(microprog[mpc]);

    cout << "\n\nMemÃ³ria principal: \nPilha: \n";
    for(int i = lv*4; i <= sp*4; i+=4)
    {
        write_byte(memory[i+3]);
        cout << " ";
        write_byte(memory[i+2]);
        cout << " ";
        write_byte(memory[i+1]);
        cout << " ";
        write_byte(memory[i]);
        cout << " : ";
        if(i < 10) cout << " ";
        cout << i << " | " << memory[i+3] << " " << memory[i+2] << " " << memory[i+1] << " " << memory[i];
        word w;
        memcpy(&w, &memory[i], 4);
        cout << " | " << i/4 << " : " << w << endl;
    }

    cout << "\n\nPC: \n";
    for(int i = (pc-1); i <= pc+20; i+=4)
    {
        write_byte(memory[i+3]);
        cout << " ";
        write_byte(memory[i+2]);
        cout << " ";
        write_byte(memory[i+1]);
        cout << " ";
        write_byte(memory[i]);
        cout << " : ";
        if(i < 10) cout << " ";
        cout << i << " | " << memory[i+3] << " " << memory[i+2] << " " << memory[i+1] << " " << memory[i];
        word w;
        memcpy(&w, &memory[i], 4);
        cout << " | " << i/4 << " : " << w << endl;
    }

    cout << "\nRegistradores - \nMAR: " << mar << " ("; write_word(mar);
    cout << ") \nMDR: " << mdr << " ("; write_word(mdr);
    cout << ") \nPC : " << pc << " ("; write_word(pc);
    cout << ") \nMBR: " << (int) mbr << " ("; write_byte(mbr);
    cout << ") \nSP : " << sp << " (";  write_word(sp);
    cout << ") \nLV : " << lv << " ("; write_word(lv);
    cout << ") \nCPP: " << cpp << " ("; write_word(cpp);
    cout << ") \nTOS: " << tos << " ("; write_word(tos);
    cout << ") \nOPC: " << opc << " ("; write_word(opc);
    cout << ") \nH  : " << h << " ("; write_word(h);
    cout << ")" << endl;
    write_byte(memory[48]);
    cout << memory[48] << endl;
    
}
//Recebe uma microinstru. binária e separa suas partes.
decoded_microcode decode_microcode(microcode code) 
{   
    decoded_microcode dec;

    dec.reg_r = code & 0b1111;
    code = code >> 4;

    dec.mem = code & 0b111;
    code = code >> 3;

    dec.reg_w = code & 0b111111111;
    code = code >> 9;

    dec.alu = code & 0b111111; // ula menos os 2 do sft
    code = code >> 6;

    dec.sft = code & 0b11; //SLL8 SRA1, os primeiros 2 da ula
    code = code >> 2;

    dec.jam = code & 0b111;
    code = code >> 3;

    dec.nadd = code & 0b111111111; //os 9 primeiros espaços para o "next-addres" da microinstrução
    code = code >> 9;

    return dec;
}

void alu(byte func, word a, word b)
{
  
    switch (func){
    case (0b011000):
        alu_out = a;
        break;
    case (0b010100):
        alu_out = b;
        break;
    case (0b011010):
        alu_out = ~a;
        break;
    case (0b101100):
        alu_out = ~b;
        break;
    case (0b111100):
        alu_out = a + b;
        break;
    case (0b111101):
        alu_out = a + b + 1;
        break;
    case (0b111001):
        alu_out = a + 1;
        break;
    case (0b110101):
        alu_out = b + 1;
        break;
    case (0b111111):
        alu_out = b - a;
        break;
    case (0b110110):
        alu_out = b - 1;
        break;
    case (0b111011):
        alu_out = -a;
        break;
    case (0b001100):
        alu_out = a & b;
        break;
    case (0b011100):
        alu_out = a | b;
        break;
    case (0b010000):
        alu_out = 0;
        break;
    case (0b110001):
        alu_out = 1;
        break;
    case (0b110010):
        alu_out = -1;
        break;
    default:
        break;
    }
    if (alu_out == 0){
        z = 1; n = 0;   
    } else {
        n = 1; z = 0;
    }
}

//Deslocamento.
void shift(byte s, word w) { 
  //considerando a inversão na hora de decodificar os bits de deslocamento "erro"

    if (s & 0b01){
        w = w << 8;
    }
    if (s & 0b10){
        w = w >> 1;
    }
    bus_c = w;
}

//Leitura de registradores.
void read_registers(byte reg_end) {   
 
    if(reg_end == 0b0000){
        bus_b = mdr;
    } else if(reg_end == 0b0001){
        bus_b = pc;
    } else if(reg_end == 0b0010){
        bus_b = mbr;
    } else if(reg_end==0b0011){
        //Retorna o valor de MBR com sinal
        //3 - carrega o MBR com sinal fazendo a extensão de sinal, ou seja, copia-se o bit mais significativo do MBR para as 24 posições mais significativas do barramento B.
        if(bus_b > 256) bus_b = bus_b | (0b111111111111111111111111 << 8); // | = ou
    } else if(reg_end == 0b0100){
        bus_b = sp;
    } else if(reg_end == 0b0101){
        bus_b = lv;
    } else if(reg_end == 0b0110){
        bus_b = cpp;
    } else if(reg_end == 0b0111){
        bus_b = tos;
    } else if(reg_end == 0b1000){
        bus_b = opc;
    } 
        bus_a = h;
    
    
}

//Escrita de registradores.
void write_register(word reg_end) {  

  if(reg_end & 0b000000001){
        mar = bus_c;
    }
	if(reg_end & 0b000000010){
        mdr = bus_c;
    }
	if(reg_end & 0b000000100){
        pc = bus_c;
    }
	if(reg_end & 0b000001000){
        sp = bus_c;
    }
	if(reg_end & 0b000010000){
        lv = bus_c;
    }
	if(reg_end & 0b000100000){
        cpp = bus_c;
    }
	if(reg_end & 0b001000000){
        tos = bus_c;
    }
	if(reg_end & 0b010000000){
        opc = bus_c;
    }
	if(reg_end & 0b100000000){
        h = bus_c;
    }
}

//Leitura e escrita de memória.
void mainmemory_io(byte control){ 
    if(control & 0b001){ // featch
        mbr = memory[pc];
    } 
    if(control & 0b010){ // read
        //mar*4 fez a conversão de word para byte) e soma 3 que é o tamanho de uma palavra
        //sim, com +3, +2, +1 e só vezes 4
        word aux; 
        aux = (memory[(mar * 4) + 3]);
        aux = aux << 8;
        aux += (memory[(mar * 4) + 2]);
        aux = aux << 8;
        aux += (memory[(mar * 4) + 1]);
        aux = aux << 8;
        aux += (memory[(mar * 4) ]);
        mdr = aux;
    //   for (int i = 3; i<1; i--){
    //     aux += (memory[(mar * 4) + i]);
    //     aux = aux << 8;
    //   }
    // for (int i = 2; i < 0; i--){
    //     aux += (memory[(mar * 4) + i]);
    //     aux = aux << 8;
    // }  
    }
    if(control & 0b100){ //white
        word mdrWord = mdr; //mdr para palavra
        for (int i = 0; i < 4; i++){
            memory[(mar * 4) + i] = (mdrWord & 0xff);
            mdrWord = mdrWord >> 8;
        }
    }
}

//Define próxima microinstru. a ser executada. 
word next_address(word next, byte jam) { 

    if(jam & 0b001){ //jamZ
        next |= z << 8;
    }
    if(jam & 0b010){//jamN
        next |= n << 8;
    }
    if(jam & 0b100){//jamPC
        next |= mbr;
    }

    return next;
}

int main(int argc, char* argv[]){
    load_microprog(); //carrega microprograma de controle

    load_prog(); //carrega programa na memória principal a ser executado pelo emulador. JÃ¡ que nÃ£o temos entrada e saÃ­da, jogamos o programa direto na memÃ³ria ao executar o emulador.

    decoded_microcode decmcode;

    while(!halt)
    {
        debug();

        decmcode = decode_microcode(microprog[mpc]);

        read_registers(decmcode.reg_r);

        alu(decmcode.alu, h, bus_b);

        bus_c = alu_out;

        shift(decmcode.sft, alu_out);

        write_register(decmcode.reg_w);

        mainmemory_io(decmcode.mem);

        mpc = next_address(decmcode.nadd, decmcode.jam);

	      getchar();

    }
    

    debug(false);

    return 0;

}