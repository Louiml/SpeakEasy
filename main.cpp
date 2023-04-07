#include <iostream>
#include <string>
#include <sapi.h>
#include <sphelper.h>
#include <lame.h>

#pragma comment(lib, "sapi.lib")
#pragma comment(lib, "libmp3lame.lib")

using namespace std;

bool WavToMp3(const char* wavFile, const char* mp3File) {
    FILE* wav = fopen(wavFile, "rb");
    FILE* mp3 = fopen(mp3File, "wb");

    if (!wav || !mp3) {
        return false;
    }

    fseek(wav, 0, SEEK_END);
    long wavSize = ftell(wav);
    fseek(wav, 0, SEEK_SET);

    short int* wavBuffer = new short int[wavSize / 2];
    fread(wavBuffer, 2, wavSize / 2, wav);

    lame_t lame = lame_init();
    lame_set_in_samplerate(lame, 16000);
    lame_set_VBR(lame, vbr_default);
    lame_init_params(lame);

    int mp3Size = 1.25 * wavSize / 2 + 7200;
    unsigned char* mp3Buffer = new unsigned char[mp3Size];

    int encodedSize = lame_encode_buffer_interleaved(lame, wavBuffer, wavSize / 4, mp3Buffer, mp3Size);
    fwrite(mp3Buffer, 1, encodedSize, mp3);

    fclose(mp3);
    fclose(wav);

    delete[] wavBuffer;
    delete[] mp3Buffer;

    lame_close(lame);

    return true;
}

int main() {
    ISpVoice* pVoice = NULL;
    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
        if (SUCCEEDED(hr)) {
            wcout << L"Enter the text you want to convert to speech: ";
            wstring input;
            getline(wcin, input);

            CSpStreamFormat wavFormat;
            CComPtr<IStream> cpWavStream;
            wavFormat.AssignFormat(SPSF_16kHz16BitMono);
            hr = SPBindToFile(L"temp.wav", SPFM_CREATE_ALWAYS, &cpWavStream, &wavFormat.FormatId(), wavFormat.WaveFormatExPtr());

            if (SUCCEEDED(hr)) {
                pVoice->SetOutput(cpWavStream, TRUE);
                pVoice->Speak(input.c_str(), SPF_DEFAULT, NULL);
                pVoice->WaitUntilDone(INFINITE);

                WavToMp3("temp.wav", "output.mp3");

                system("output.mp3");
                a
                remove("temp.wav");
                remove("output.mp3");

                cpWavStream.Release();
            }
            pVoice->Release();
            pVoice = NULL;
        }
        CoUninitialize();
    }

    return 0;
}
