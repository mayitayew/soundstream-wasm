#include "encode_and_decode_lib.h"

int main(int argc, char* argv[]) {
  chromemedia::codec::End2End("48khz_playback.wav",
                              "48khz_playback.wav", argv[0]);
  return 0;
}