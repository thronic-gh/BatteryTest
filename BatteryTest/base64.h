#pragma once

//
//	Struktur for lagring av fildata.
//
struct FileData {
	unsigned long FileSize = 0;
	unsigned char* FileBytes = 0;
};

//
//	Klasse for å lese og skrive filer binært.
//
class djFileBytes 
{
	public:
	static void ReadFileBytes(const char* filename, FileData* fd)
	{
		FILE* fptr;
		size_t fres;

		// Åpne fil i binær lesemodus.
		fptr = fopen(filename, "rb");
		if (fptr == 0) {
			GetError(L"ReadFileBytes feilet å åpne filen.");
			exit(1);
		}

		// Filstørrelse.
		fseek(fptr, 0, SEEK_END);
		fd->FileSize = ftell(fptr);
		rewind(fptr);

		// Reserver minne til å holde fildata.
		fd->FileBytes = (unsigned char*) malloc(sizeof(unsigned char) * fd->FileSize);
		if (fd->FileBytes == 0) {
			GetError(L"ReadFileBytes feilet å reservere minne.");
			exit(1);
		}

		// Kopier fila inn til minnet.
		fres = fread(fd->FileBytes, 1, fd->FileSize, fptr);
		if (fres != fd->FileSize) {
			GetError(L"ReadFileBytes feilet å lese filen til minnet.");
			exit(1);
		}

		fclose(fptr);
	}

	static void WriteFileBytes(FileData* fd, const char* filename)
	{
		FILE* fptr;
		size_t fres;

		// Åpne fil i binær skrivemodus.
		fptr = fopen(filename, "wb");
		if (fptr == 0) {
			GetError(L"WriteFileBytes feilet å åpne filen.");
			exit(1);
		}

		fres = fwrite(fd->FileBytes, 1, fd->FileSize, fptr);
		fclose(fptr);
	}
};




//
//	Klasse for å enkode og dekode bytes med Base64.
//
class djBase64 
{
	public:

	// fd = FileData struktur data hentes fra.
	static std::string Encode(FileData* fd)
	{	
		// Base64 alfabet.
		char base64_table[64] = {
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
			'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
			'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
			'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
			'w', 'x', 'y', 'z', '0', '1', '2', '3', 
			'4', '5', '6', '7', '8', '9', '+', '/'
		};
		
		// Hent alle bits fra bytes.
		std::string bits;
		for (unsigned long a=0; a<fd->FileSize; a++) {
			std::bitset<8> b(fd->FileBytes[a]);
			bits.append(b.to_string());
		}
		
		//std::ofstream _debug ("encode_orig_bit_string.txt");
		//_debug.write(bits.c_str(), bits.length());
	
		// Del de opp i 6-bit deler og lag en base64 streng.
		std::string base64str = "";
		std::string sixbit = "";
		int c;

		// Håndter komplette 6-bit sekvenser.
		for (unsigned int a=0; a<bits.length(); a++) {
			if (sixbit.length() == 6) {
				c = std::stoi(sixbit,nullptr,2);
				base64str.push_back(base64_table[c]);
				sixbit.clear();
			}
			sixbit.append(bits.substr(a,1));	
		}

		// Håndter gjenstående bits.
		if (sixbit.length() > 0) {
			while (sixbit.length() < 6)
				sixbit.push_back('0');
			c = std::stoi(sixbit,nullptr,2);
			base64str.push_back(base64_table[c]);
		}
	
		//
		// Legg på = padding hvis strengen ikke er delbar med 4.
		// Det trengs 4 base64 karakterer for å dekode 3 bytes.
		// Det trengs 3 base64 karakterer for å dekode 2 bytes.
		// Det trengs 2 base64 karakterer for å dekode 1 byte.
		// Dette er fordi 1x 8-bit tar 2x 6-bit lengder.
		// Ved dekoding vet man ved hjelp av padding hvor mange 
		// bytes det er på slutten av en streng.
		//
		while (base64str.length() % 4)
			base64str.push_back('=');

		return base64str;
	}


	// Base64String = String som skal dekodes.
	// fd = FileData struktur data kan lagres til.
	static void Decode(std::string Base64String, FileData *fd)
	{
		// Base64 alfabet.
		char base64_table[64] = {
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
			'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
			'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
			'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
			'w', 'x', 'y', 'z', '0', '1', '2', '3', 
			'4', '5', '6', '7', '8', '9', '+', '/'
		};

		// Hent alle bits til en lang streng, unntatt =.
		std::string bits;
		for (size_t a=0; a<Base64String.length(); a++) {

			if (Base64String[a] == '=')
				break;

			for (int b=0; b<64; b++) {
				if (Base64String[a] == base64_table[b]) {
					std::bitset<6> sixbits(b);
					bits.append(sixbits.to_string());
				}
			}
		}

		// Fjern evt. 0 padding utført på slutten.
		while (bits.length() % 8)
			bits.pop_back();

		//std::ofstream _debug ("decode_orig_bit_string.txt");
		//_debug.write(bits.c_str(), bits.length());

		// Bit-streng bør nå være klar for dekoding.
		std::string eightbit = "";
		std::vector<unsigned char> c;
		for (size_t a=0; a<bits.length()+1; a++) {

			// bits.length()+1 gjør at alle bits blir sjekket.
			if (eightbit.length() == 8) {
				c.push_back(std::stoi(eightbit,nullptr,2));
				eightbit.clear();
			}

			// Ikke gå over indeks.
			if (a<bits.length())
				eightbit.push_back(bits[a]);
		}

		// Skriv bytes til pekt datastruktur.
		fd->FileSize = c.size();
		fd->FileBytes = (unsigned char*) malloc(sizeof(unsigned char) * fd->FileSize);
		for (size_t a=0; a<c.size(); a++)
			fd->FileBytes[a] = c[a];
	}
};