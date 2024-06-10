#include <iostream>
#include <vector>
#include <fstream>
#include <locale>
#include <codecvt>
#include <bitset>
#include <sndfile.h>
using namespace std;

locale loc(locale(), new codecvt_utf8<wchar_t>);
//-------------------------------------------------------------------------------------------
//ESTRUCTURAS PARA LA CREACION DEL ARBOL DE HUFFMAN
//-------------------------------------------------------------------------------------------
struct indice_frecuencia {
    wchar_t caracter;
    int frecuencia;
};
struct indice_codigo {
    wchar_t indice;
    wstring codigo;
};
struct nodo {
    wchar_t letra;
    int cantidad;
    wstring codigo;
    struct nodo* sig;
    struct nodo* izq, * der;
};
typedef struct nodo* ArbolH;

//-------------------------------------------------------------------------------------------
//FUNCIONES PARA LA CREACION DEL ARBOL DE HUFFMAN
//-------------------------------------------------------------------------------------------
ArbolH crear_nodo(wchar_t x, int y) {
    ArbolH nuevo_nodo = new struct nodo;
    nuevo_nodo->letra = x;
    nuevo_nodo->cantidad = y;
    nuevo_nodo->codigo = L"\0";
    nuevo_nodo->sig = NULL;
    nuevo_nodo->izq = NULL;
    nuevo_nodo->der = NULL;
    return nuevo_nodo;
}
void insertar_nodo(ArbolH& arbol, wchar_t x, int y) {
    if (arbol == NULL) {
        arbol = crear_nodo(x, y);
    }
    else {
        ArbolH temp1 = crear_nodo(x, y);
        ArbolH temp2 = arbol;
        while (temp2->sig != NULL) {
            temp2 = temp2->sig;
        }
        temp2->sig = temp1;
    }
}
void formar_arbol(ArbolH& arbol) {
    while (arbol != NULL && arbol->sig != NULL) {
        ArbolH temp1 = crear_nodo('\0', arbol->cantidad + arbol->sig->cantidad);
        temp1->izq = arbol;
        temp1->der = arbol->sig;
        arbol = arbol->sig->sig;
        if (arbol == NULL) {
            arbol = temp1;
        }
        else if (temp1->cantidad <= arbol->cantidad) {
            temp1->sig = arbol;
            arbol = temp1;
        }
        else {
            ArbolH temp2 = arbol;
            while (temp2->sig != NULL && temp2->sig->cantidad < temp1->cantidad) {
                temp2 = temp2->sig;
            }
            temp1->sig = temp2->sig;
            temp2->sig = temp1;
        }
    }
}
void codificar(ArbolH arbol, wstring codigo_actual, vector<indice_codigo>& codigos) {
    if (arbol == NULL) {
        return;
    }
    arbol->codigo = codigo_actual;
    if (arbol->letra != '\0') {
        indice_codigo aux;
        aux.indice = arbol->letra;
        aux.codigo = arbol->codigo;
        codigos.push_back(aux);
    }
    codificar(arbol->izq, codigo_actual + L"0", codigos);
    codificar(arbol->der, codigo_actual + L"1", codigos);
}
void guardar_arbol(wofstream& arbol, vector<indice_codigo> codigos, string& semilla) {
    for (int i = 0; i < codigos.size(); i++) {
        arbol << codigos[i].codigo << codigos[i].indice;
    }
    int aumen = 0;
    while (semilla.length() < 32) {
        int aux_bit = codigos[aumen].codigo.length() - 1;
        semilla += codigos[aumen].codigo[aux_bit];
        aumen++;
        if (aumen == codigos.size()) {
            aumen = 0;
        }
    }
}
void guardar_texto_codificado(wifstream& txtEntrada, vector<indice_codigo> codigos, wstring& txtCodificado) {
    wchar_t c;
    while (txtEntrada.get(c)) {
        for (int i = 0; i < codigos.size(); i++) {
            if (c == codigos[i].indice) {
                txtCodificado += codigos[i].codigo;
                break;
            }
        }
    }
    int indice_relleno;
    for (int i = 0; i < codigos.size(); i++) {
        if (codigos[i].indice == L'´') {
            indice_relleno = i;
            break;
        }
    }
    while (txtCodificado.length() % 128 != 0) {
        txtCodificado += codigos[indice_relleno].codigo;
    }
}
void eliminar_arbol(ArbolH& nodo) {
    if (nodo != NULL) {
        eliminar_arbol(nodo->izq);
        eliminar_arbol(nodo->der);
        delete nodo;
    }
}
int repetido(wchar_t caracter, vector<indice_frecuencia>& lista) {
    for (int i = 0; i < lista.size(); i++) {
        if (lista[i].caracter == caracter) {
            return i;
        }
    }
    return -1;
}
void ordenar_lista(vector<indice_frecuencia>& lista, int& frec_max) {
    indice_frecuencia aux;
    for (int i = 0; i < lista.size(); i++) {
        for (int j = 0; j < lista.size() - 1 - i; j++) {
            if (lista[j].frecuencia > lista[j + 1].frecuencia) {
                aux = lista[j];
                lista[j] = lista[j + 1];
                lista[j + 1] = aux;
            }
        }
    }
    for (int i = 0; i < lista.size(); i++) {
        frec_max += lista[i].frecuencia;
    }
}

void arbol_huffman(wstring& textoCodificado, string& semilla, string& direc) {
    vector<indice_frecuencia> letras_texto; string direccion; wchar_t caract;
    cout << "Ingrese la direccion del archivo de texto a ocultar: "; cin >> direccion;
    wifstream txtHuffmanEntrada(direccion);
    txtHuffmanEntrada.imbue(loc);
    while (txtHuffmanEntrada.get(caract)) {
        int indice = repetido(caract, letras_texto);
        if (indice != -1) {
            letras_texto[indice].frecuencia += 1;
        }
        else {
            indice_frecuencia letra;
            letra.caracter = caract;
            letra.frecuencia = 1;
            letras_texto.push_back(letra);
        }
    }
    int frec_max = 0;
    ordenar_lista(letras_texto, frec_max);
    indice_frecuencia letra_aux; letra_aux.caracter = L'´'; letra_aux.frecuencia = frec_max + 1;
    letras_texto.push_back(letra_aux);
    ArbolH arbol = NULL;
    for (int i = 0; i < letras_texto.size(); i++) {
        insertar_nodo(arbol, letras_texto[i].caracter, letras_texto[i].frecuencia);
    }
    formar_arbol(arbol);
    vector<indice_codigo> codigos;
    wstring codigo_actual = L"";
    codificar(arbol, codigo_actual, codigos);
    cout << "Ingrese la direccion donde se guardara el arbol generado: "; cin >> direc;
    wofstream arbolTxt(direc);
    arbolTxt.imbue(loc);
    guardar_arbol(arbolTxt, codigos, semilla);
    txtHuffmanEntrada.clear();
    txtHuffmanEntrada.seekg(0);
    guardar_texto_codificado(txtHuffmanEntrada, codigos, textoCodificado);
    arbolTxt.close();
    txtHuffmanEntrada.close();
    eliminar_arbol(arbol);
    wofstream pre_binario("D:\\VScode\\ED2\\TIF_LSB\\TIF_LSB\\Salida datos\\pre_code_binario.txt");
    pre_binario.imbue(loc);
    pre_binario << textoCodificado;
    pre_binario.close();
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
void pre_codificar(wstring& textoCodificado, vector<unsigned char>& hexas) {
    string cadenaBits;
    int avance = 0;
    for (int i = 0; i < textoCodificado.length(); i++) {
        cadenaBits += textoCodificado[i];
        avance++;
        if (avance == 8) {
            unsigned char resultado = convertirBitsAHex(cadenaBits);
            hexas.push_back(resultado);
            cadenaBits.clear();
            avance = 0;
        }
    }
    ofstream hexas_pre_code("D:\\VScode\\ED2\\TIF_LSB\\TIF_LSB\\Salida datos\\hexas_pre_code.txt");
    hexas_pre_code.imbue(loc);
    for (int i = 0; i < hexas.size(); i++) {
        hexas_pre_code << "Valor hex " << i + 1 << ": " << hex << static_cast<int>(hexas[i]) << endl;
    }
    hexas_pre_code.close();
}
//-------------------------------------------------------------------------------------------
//FUNCIÓN PARA TRANSFORMAR UN HEX A 8-BITS
//-------------------------------------------------------------------------------------------
string convertirHexABits(unsigned char valor) {
    bitset<8> bits(valor);
    string bitsStr = bits.to_string();
    return bitsStr;
}
void post_codificar(vector<unsigned char>& hexas_codif, string& codificado_binario) {
    for (int i = 0; i < hexas_codif.size(); i++) {
        unsigned char valorHex = hexas_codif[i];
        string bits_aux = convertirHexABits(valorHex);
        codificado_binario += bits_aux;
    }
    ofstream post_binario("D:\\VScode\\ED2\\TIF_LSB\\TIF_LSB\\Salida datos\\post_code_binario.txt");
    post_binario.imbue(loc);
    post_binario << codificado_binario;
    post_binario.close();
}
//-------------------------------------------------------------------------------------------
//FUNCIONES PARA CODIFICAR MEDIANTE RIJNDAEL
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
void Sub_Bytes() {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            state[i][j] = get_SBox_Value(state[i][j]);

        }
    }
}
void Shift_Rows() {
    unsigned char temp;

    temp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = temp;

    temp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = temp;
    temp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = temp;

    temp = state[3][0];
    state[3][0] = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = temp;
}
void Mix_Columns() {
    int i;
    unsigned char x1, x2, x3;
    for (i = 0; i < 4; i++) {
        x1 = state[0][i];
        x3 = state[0][i] ^ state[1][i] ^ state[2][i] ^ state[3][i];
        x2 = state[0][i] ^ state[1][i]; x2 = xtime(x2); state[0][i] ^= x2 ^ x3;
        x2 = state[1][i] ^ state[2][i]; x2 = xtime(x2); state[1][i] ^= x2 ^ x3;
        x2 = state[2][i] ^ state[3][i]; x2 = xtime(x2); state[2][i] ^= x2 ^ x3;
        x2 = state[3][i] ^ x1; x2 = xtime(x2); state[3][i] ^= x2 ^ x3;
    }
}
void Encrypt() {
    int i, j, round = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            state[j][i] = plaintext[i * 4 + j];
        }
    }
    Add_Round_Key(0);
    for (round = 1; round < rounds; round++) {
        Sub_Bytes();
        Shift_Rows();
        Mix_Columns();
        Add_Round_Key(round);
    }
    Sub_Bytes();
    Shift_Rows();
    Add_Round_Key(rounds);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            encrypted[i * 4 + j] = state[j][i];
        }
    }
}
void codificar(vector<unsigned char>& hexas, vector<unsigned char>& hexas_codif) {
    int i;
    while (rounds != 128 && rounds != 192 && rounds != 256) {
        printf("Tamaño de la clave (en bits)? (128, 192 or 256): ");
        scanf_s("%d", &rounds);
    }
    keyLength = rounds / 32;
    rounds = keyLength + 6;
    _flushall();
    printf("Introduzca la clave en Hexadecimales: ");
    for (i = 0; i < keyLength * 4; i++) {
        scanf_s("%x", &Key[i]);
    }
    for (int j = 0; j < hexas.size(); j += 16) {
        for (i = 0; i < blockSize * 4; i++) {
            plaintext[i] = hexas[i + j];
        }
        Expand_Keys();
        Encrypt();
        for (i = 0; i < keyLength * blockSize; i++) {
            hexas_codif.push_back(encrypted[i]);
        }
        _flushall();
    }
    ofstream hexas_post_code("D:\\VScode\\ED2\\TIF_LSB\\TIF_LSB\\Salida datos\\hexas_post_code.txt");
    hexas_post_code.imbue(loc);
    for (int i = 0; i < hexas_codif.size(); i++) {
        hexas_post_code << "Valor hex " << i + 1 << ": " << hex << static_cast<int>(hexas_codif[i]) << endl;
    }
    hexas_post_code.close();
}
//-------------------------------------------------------------------------------------------
//FUNCIONES PARA OCULTAR BITS EN AUDIO
//-------------------------------------------------------------------------------------------
void modificarBits(short& muestra, char primer_bit, char segundo_bit) {
    bitset<16> bitsMuestra(muestra);
    if (primer_bit == '0') {
        bitsMuestra.reset(1);
    }
    else if (primer_bit == '1') {
        bitsMuestra.set(1);
    }
    if (segundo_bit == '0') {
        bitsMuestra.reset(0);
    }
    else if (segundo_bit == '1') {
        bitsMuestra.set(0);
    }
    muestra = static_cast<short>(bitsMuestra.to_ulong());
}
bool numeroYaGenerado(const vector<int>& numerosGenerados, int numero) {
    for (int num : numerosGenerados) {
        if (num == numero) {
            return true;
        }
    }
    return false;
}
void ocultar_datos(string& codificado_binario, string& semilla, string& direc) {
    bitset<32> seedBits(semilla);
    unsigned int seed = seedBits.to_ulong();
    srand(seed);

    SF_INFO info;
    SNDFILE* archivoEntrada = sf_open("guitarra.wav", SFM_READ, &info);
    if (!archivoEntrada) {
        cerr << "Error al abrir el archivo de audio de entrada 1" << endl;
        return;
    }
    SF_INFO info_salida = info;
    SNDFILE* archivoSalida = sf_open("guitarra2.wav", SFM_WRITE, &info_salida);
    if (!archivoEntrada) {
        cerr << "Error al abrir el archivo de audio de entrada 2" << endl;
        return;
    }
    int totalMuestras = info.frames * info.channels;

    const int rangoMinimo = 0;
    const int rangoMaximo = totalMuestras;
    const int cantidadNumeros = codificado_binario.length() / 2;
    vector<int> numerosGenerados;
    while (numerosGenerados.size() < cantidadNumeros) {
        int numero = rand() % (rangoMaximo - rangoMinimo + 1) + rangoMinimo;
        if (!numeroYaGenerado(numerosGenerados, numero)) {
            numerosGenerados.push_back(numero);
        }
    }
    ofstream numerosg("D:\\VScode\\ED2\\TIF_LSB\\TIF_LSB\\Salida datos\\PosicionesGeneradasOcultar.txt"); wofstream huff(direc, ios::app);
    huff.imbue(loc);
    for (int i = 0; i < numerosGenerados.size(); i++) {
        numerosg << numerosGenerados[i] << " ";
    }
    numerosg.close();
    huff << L'`' << cantidadNumeros;
    huff.close();

    short* buffer = new short[totalMuestras];
    sf_readf_short(archivoEntrada, buffer, info.frames);
    int j = 0;
    for (int i = 0; i < numerosGenerados.size(); i++) {
        modificarBits(buffer[numerosGenerados[i]], codificado_binario[j], codificado_binario[j + 1]);
        j += 2;
    }
    sf_writef_short(archivoSalida, buffer, info.frames);
    delete[] buffer;
    sf_close(archivoEntrada);
    sf_close(archivoSalida);
    cout << "La informacion ha sido ocultada exitosamente." << endl;
}
int main() {
    wstring textoCodificado; vector<unsigned char> hexas, hexas_codif; string codificado_binario, semilla, direc;
    arbol_huffman(textoCodificado, semilla, direc);
    pre_codificar(textoCodificado, hexas);
    codificar(hexas, hexas_codif);
    post_codificar(hexas_codif, codificado_binario);
    ocultar_datos(codificado_binario, semilla, direc);
    return 0;
}