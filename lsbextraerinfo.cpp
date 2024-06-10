#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <locale>
#include <codecvt>
#include <bitset>
#include <sndfile.h>
using namespace std;

locale loc(locale(), new codecvt_utf8<wchar_t>);
//-------------------------------------------------------------------------------------------
//FUNCIÓN PARA EXTRAER BITS DEL AUDIO
//-------------------------------------------------------------------------------------------
void extraerBits(short muestra, string& textoCodificado) {
    bitset<16> bitsMuestra(muestra);
    textoCodificado += bitsMuestra[1] ? '1' : '0';
    textoCodificado += bitsMuestra[0] ? '1' : '0';
}
bool numeroYaGenerado(const vector<int>& numerosGenerados, int numero) {
    for (int num : numerosGenerados) {
        if (num == numero) {
            return true;
        }
    }
    return false;
}
void extraer_datos_audio(string& direc, string &textoCodeBinario) {
    cout << "Ingrese la direccion del archivo con el arbol guardado: "; cin >> direc;
    wifstream arbolhuff(direc); wchar_t car;
    arbolhuff.imbue(loc);
    string semilla, cantidad_n; wstring code_aux, reemplazo;
    while (arbolhuff.get(car)) {
        code_aux += car;
    }
    for (int i = 0; i < code_aux.length(); i++) {
        if (semilla.length() < 32) {
            if (code_aux[i] != L'1' && code_aux[i] != L'0') {
                semilla += code_aux[i - 1];
            }
            if (i == code_aux.length() - 1) {
                i = 0;
            }
        }
        if (code_aux[i] == L'`') {
            while (i < code_aux.length() - 1) {
                i++;
                cantidad_n += code_aux[i];
            }
        }
        if (i < code_aux.length() - 1) {
            reemplazo += code_aux[i];
        }
    }
    arbolhuff.close();
    wofstream arbolhuffm(direc);
    arbolhuffm.imbue(loc);
    arbolhuffm << reemplazo;
    arbolhuffm.close();
    int cantidad_n_int = stoi(cantidad_n);

    bitset<32> seedBits(semilla);
    unsigned int seed = seedBits.to_ulong();
    srand(seed);

    SF_INFO info;
    SNDFILE* archivoEntrada = sf_open("guitarra2.wav", SFM_READ, &info);
    if (!archivoEntrada) {
        cerr << "Error al abrir el archivo de audio de entrada" << endl;
        return;
    }
    int totalMuestras = info.frames * info.channels;
    short* buffer = new short[totalMuestras];
    sf_readf_short(archivoEntrada, buffer, info.frames);

    const int rangoMinimo = 0;
    const int rangoMaximo = totalMuestras;
    const int cantidadNumeros = cantidad_n_int;
    vector<int> numerosGenerados;
    while (numerosGenerados.size() < cantidadNumeros) {
        int numero = rand() % (rangoMaximo - rangoMinimo + 1) + rangoMinimo;
        if (!numeroYaGenerado(numerosGenerados, numero)) {
            numerosGenerados.push_back(numero);
        }
    }
    ofstream numerosg("D:\\VScode\\ED2\\LSB_TIF_2\\LSB_TIF_2\\Salida datos\\PosicionesGeneradasExtraer.txt");
    for (int i = 0; i < numerosGenerados.size(); i++) {
        numerosg << numerosGenerados[i] << " ";
    }
    numerosg.close();

    int j = 0;
    for (int i = 0; i < numerosGenerados.size(); i++) {
        extraerBits(buffer[numerosGenerados[i]], textoCodeBinario);
        j += 2;
    }
    delete[] buffer;
    sf_close(archivoEntrada);
    ofstream txtcode("D:\\VScode\\ED2\\LSB_TIF_2\\LSB_TIF_2\\Salida datos\\TextoCodificadoBinarioExtraido.txt");
    txtcode << textoCodeBinario;
    txtcode.close();
}
//-------------------------------------------------------------------------------------------
//FUNCIÓN PARA TRANSFORMAR 8-BITS EN HEX
//-------------------------------------------------------------------------------------------
unsigned char convertirBitsAHex(const string& bits) {
    unsigned char valor = 0;
    for (size_t i = 0; i < 8; ++i) {
        valor <<= 1;
        valor |= (bits[i] - '0');
    }

    return valor;
}
void pre_decodificar(string& codificado_binario, vector<unsigned char>& hexas_codif) {
    string cadenaBits;
    int avance = 0;
    for (int i = 0; i < codificado_binario.length(); i++) {
        cadenaBits += codificado_binario[i];
        avance++;
        if (avance == 8) {
            unsigned char resultado = convertirBitsAHex(cadenaBits);
            hexas_codif.push_back(resultado);
            cadenaBits.clear();
            avance = 0;
        }
    }
    ofstream hexas_pre_decode("D:\\VScode\\ED2\\LSB_TIF_2\\LSB_TIF_2\\Salida datos\\hexas_pre_decode.txt");
    hexas_pre_decode.imbue(loc);
    for (int i = 0; i < hexas_codif.size(); i++) {
        hexas_pre_decode << "Valor hex " << i + 1 << ": " << hex << static_cast<int>(hexas_codif[i]) << endl;
    }
    hexas_pre_decode.close();
}
//-------------------------------------------------------------------------------------------
//FUNCIÓN PARA TRANSFORMAR UN HEX A 8-BITS
//-------------------------------------------------------------------------------------------
string convertirHexABits(unsigned char valor) {
    bitset<8> bits(valor);
    string bitsStr = bits.to_string();
    return bitsStr;
}
void post_decodificar(vector<unsigned char>& hexas_decodif, string& decodificado_binario) {
    for (int i = 0; i < hexas_decodif.size(); i++) {
        unsigned char valorHex = hexas_decodif[i];
        string bits_aux = convertirHexABits(valorHex);
        decodificado_binario += bits_aux;
    }
    ofstream post_binario("D:\\VScode\\ED2\\LSB_TIF_2\\LSB_TIF_2\\Salida datos\\post_decode_binario.txt");
    post_binario.imbue(loc);
    post_binario << decodificado_binario;
    post_binario.close();
}
//-------------------------------------------------------------------------------------------
//FUNCIONES PARA DECODIFICAR MEDIANTE RIJNDAEL
//-------------------------------------------------------------------------------------------
#define blockSize 4 
#define xtime(x)   ((x<<1) ^ (((x>>7) & 1) * 0x1b))
#define Multiply(x,y) (((y & 1) * x) ^ ((y>>1 & 1) * xtime(x)) ^ ((y>>2 & 1) * xtime(xtime(x))) ^ ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^ ((y>>4 & 1) * xtime(xtime(xtime(xtime(x))))))

int rounds = 0;
int keyLength = 0;
unsigned char plaintext[16], encrypted[16], state[4][4];
unsigned char roundKey[240], Key[32];

int get_SBox_Value(int num) {
    int sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
    };
    return sbox[num];
}
int get_SBox_Inverse(int num) {
    int rsbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
    };
    return rsbox[num];
}
int Rcon[255] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
    0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39,
    0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
    0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
    0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
    0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
    0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b,
    0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
    0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
    0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
    0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
    0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
    0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
    0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63,
    0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd,
    0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb };
void Expand_Keys() {
    int i, j;
    unsigned char temp[4], k;
    for (i = 0; i < keyLength; i++) {
        roundKey[i * 4] = Key[i * 4];
        roundKey[i * 4 + 1] = Key[i * 4 + 1];
        roundKey[i * 4 + 2] = Key[i * 4 + 2];
        roundKey[i * 4 + 3] = Key[i * 4 + 3];
    }
    while (i < (blockSize * (rounds + 1))) {
        for (j = 0; j < 4; j++) {
            temp[j] = roundKey[(i - 1) * 4 + j];
        }
        if (i % keyLength == 0) {
            {
                k = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = k;
            }
            {
                temp[0] = get_SBox_Value(temp[0]);
                temp[1] = get_SBox_Value(temp[1]);
                temp[2] = get_SBox_Value(temp[2]);
                temp[3] = get_SBox_Value(temp[3]);
            }
            temp[0] = temp[0] ^ Rcon[i / keyLength];
        }
        else if (keyLength > 6 && i % keyLength == 4) {
            {
                temp[0] = get_SBox_Value(temp[0]);
                temp[1] = get_SBox_Value(temp[1]);
                temp[2] = get_SBox_Value(temp[2]);
                temp[3] = get_SBox_Value(temp[3]);
            }
        }
        roundKey[i * 4 + 0] = roundKey[(i - keyLength) * 4 + 0] ^ temp[0];
        roundKey[i * 4 + 1] = roundKey[(i - keyLength) * 4 + 1] ^ temp[1];
        roundKey[i * 4 + 2] = roundKey[(i - keyLength) * 4 + 2] ^ temp[2];
        roundKey[i * 4 + 3] = roundKey[(i - keyLength) * 4 + 3] ^ temp[3];
        i++;
    }
}
void Add_Round_Key(int round) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            state[j][i] ^= roundKey[round * blockSize * 4 + i * blockSize + j];
        }
    }
}
void Inv_Sub_Bytes() {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            state[i][j] = get_SBox_Inverse(state[i][j]);

        }
    }
}
void Inv_Shift_Rows() {
    unsigned char temp;

    temp = state[1][3];
    state[1][3] = state[1][2];
    state[1][2] = state[1][1];
    state[1][1] = state[1][0];
    state[1][0] = temp;

    temp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = temp;
    temp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = temp;

    temp = state[3][0];
    state[3][0] = state[3][1];
    state[3][1] = state[3][2];
    state[3][2] = state[3][3];
    state[3][3] = temp;
}
void Inv_Mix_Columns() {
    int i;
    unsigned char x1, x2, x3, x4;
    for (i = 0; i < 4; i++) {
        x1 = state[0][i];
        x2 = state[1][i];
        x3 = state[2][i];
        x4 = state[3][i];

        state[0][i] = Multiply(x1, 0x0e) ^ Multiply(x2, 0x0b) ^ Multiply(x3, 0x0d) ^ Multiply(x4, 0x09);
        state[1][i] = Multiply(x1, 0x09) ^ Multiply(x2, 0x0e) ^ Multiply(x3, 0x0b) ^ Multiply(x4, 0x0d);
        state[2][i] = Multiply(x1, 0x0d) ^ Multiply(x2, 0x09) ^ Multiply(x3, 0x0e) ^ Multiply(x4, 0x0b);
        state[3][i] = Multiply(x1, 0x0b) ^ Multiply(x2, 0x0d) ^ Multiply(x3, 0x09) ^ Multiply(x4, 0x0e);
    }
}
void Decrypt() {
    int i, j, round = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            state[j][i] = encrypted[i * 4 + j];
        }
    }
    Add_Round_Key(rounds);
    for (round = rounds - 1; round > 0; round--) {
        Inv_Shift_Rows();
        Inv_Sub_Bytes();
        Add_Round_Key(round);
        Inv_Mix_Columns();
    }
    Inv_Shift_Rows();
    Inv_Sub_Bytes();
    Add_Round_Key(0);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            plaintext[i * 4 + j] = state[j][i];
        }
    }
}
void decodificar_hex(vector<unsigned char>& hexas_codif, vector<unsigned char>& hexas_decodif) {
    int i, tam_aux;
    while (rounds != 128 && rounds != 192 && rounds != 256) {
        printf("Tamaño de la clave (en bits)? (128, 192 or 256): ");
        scanf_s("%d", &rounds);
    }
    tam_aux = rounds / 8;
    keyLength = rounds / 32;
    rounds = keyLength + 6;
    _flushall();
    printf("Introduzca la clave en Hexadecimales: ");
    for (i = 0; i < keyLength * 4; i++) {
        scanf_s("%x", &Key[i]);
    }
    for (int j = 0; j < hexas_codif.size(); j += tam_aux) {
        for (i = 0; i < blockSize * 4; i++) {
            encrypted[i] = hexas_codif[i + j];
        }
        Expand_Keys();
        Decrypt();
        for (i = 0; i < blockSize * 4; i++) {
            hexas_decodif.push_back(plaintext[i]);
        }
    }
    ofstream hexas_post_decode("D:\\VScode\\ED2\\LSB_TIF_2\\LSB_TIF_2\\Salida datos\\hexas_post_decode.txt");
    hexas_post_decode.imbue(loc);
    for (int i = 0; i < hexas_decodif.size(); i++) {
        hexas_post_decode << "Valor hex " << i + 1 << ": " << hex << static_cast<int>(hexas_decodif[i]) << endl;
    }
    hexas_post_decode.close();
}
//-------------------------------------------------------------------------------------------
//ESTRUCTURA PARA RECONSTRUIR EL ARBOL DE HUFFMAN
//-------------------------------------------------------------------------------------------
struct nodo {
    wchar_t letra;
    struct nodo* izq, * der;
};
typedef struct nodo* ArbolH;

//-------------------------------------------------------------------------------------------
//FUNCIONES PARA RECONSTRUIR EL ARBOL DE HUFFMAN Y DECODIFICAR EL MENSAJE
//-------------------------------------------------------------------------------------------
ArbolH crear_nodo(wchar_t x) {
    ArbolH nuevo_nodo = new struct nodo;
    nuevo_nodo->letra = x;
    nuevo_nodo->izq = NULL;
    nuevo_nodo->der = NULL;
    return nuevo_nodo;
}
void decodificar(ArbolH arbol, ArbolH arbol_principal, string& texto_leer, wofstream& texto_salida, char c, int& indice) {
    while (indice < texto_leer.length()) {
        if (c == L'0') {
            ArbolH aux = arbol->izq;
            if (aux->letra == L'*') {
                c = texto_leer[++indice];
                decodificar(aux, arbol_principal, texto_leer, texto_salida, c, indice);
            }
            else {
                texto_salida << aux->letra;
                c = texto_leer[++indice];
                decodificar(arbol_principal, arbol_principal, texto_leer, texto_salida, c, indice);
            }
        }
        else if (c == L'1') {
            ArbolH aux = arbol->der;
            if (aux->letra == L'*') {
                c = texto_leer[++indice];
                decodificar(aux, arbol_principal, texto_leer, texto_salida, c, indice);
            }
            else if (aux->letra != L'´') {
                texto_salida << aux->letra;
                c = texto_leer[++indice];
                decodificar(arbol_principal, arbol_principal, texto_leer, texto_salida, c, indice);
            }
            else {
                c = texto_leer[++indice];
                decodificar(arbol_principal, arbol_principal, texto_leer, texto_salida, c, indice);
            }
        }
    }
    texto_salida.close();
}
void recuperar_arbol(wifstream& texto, ArbolH& arbol) {
    wchar_t c;
    ArbolH aux = arbol;
    while (texto.get(c)) {
        if (c == L'0') {
            if (arbol->izq == NULL) {
                arbol->izq = crear_nodo(L'*');
            }
            arbol = arbol->izq;
        }
        else if (c == L'1') {
            if (arbol->der == NULL) {
                arbol->der = crear_nodo(L'*');
            }
            arbol = arbol->der;
        }
        else {
            arbol->letra = c;
            arbol = aux;
        }
    }
    arbol = aux;
}
void liberar_memoria(ArbolH& nodo) {
    if (nodo != NULL) {
        liberar_memoria(nodo->izq);
        liberar_memoria(nodo->der);
        delete nodo;
    }
}
void decodificar_huffman(string& decodificado_binario, string& direc) {
    string direccion;
    ArbolH arbol = crear_nodo(L'*');
    wifstream arbolGuardado(direc);
    arbolGuardado.imbue(loc);
    recuperar_arbol(arbolGuardado, arbol);
    arbolGuardado.close();
    cout << "Ingrese la direccion donde ira el texto decodificado: "; cin >> direccion;
    wofstream txtDecodificado(direccion);
    txtDecodificado.imbue(loc);
    char c;
    c = decodificado_binario[0];
    int indi = 0;
    decodificar(arbol, arbol, decodificado_binario, txtDecodificado, c, indi);
    liberar_memoria(arbol);
}
int main() {
    string codificado_binario, decodificado_binario, direc; vector<unsigned char> hexas_codif, hexas_decodif;
    extraer_datos_audio(direc, codificado_binario);
    pre_decodificar(codificado_binario, hexas_codif);
    decodificar_hex(hexas_codif, hexas_decodif);
    post_decodificar(hexas_decodif, decodificado_binario);
    decodificar_huffman(decodificado_binario, direc);
}