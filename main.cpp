#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "lz.h"

const uint8_t src_text[] = R"(
                                                                            
     The  city  of  the cores was gone  now,  obscured  entirely by the dark
beneath them.
     "What is it?"
     "An Al's defense system," the construct said, "or part of it.  If
it's your pal Wintermute, he's not lookin' real friendly."
     "Take it," Case said. "You're faster."
     "Now your best de-fense, boy, it's a good off-fense."
     And  the  Flatline  aligned the  nose  of  Kuang's sting with the
center of the dark below. And dove.
     Case's sensory input warped with their velocity.
     His mouth filled with an aching taste of blue.
     His  eyes  were  eggs of unstable crystal,  vibrating with  a frequency
whose  name was rain and the sound of trains, suddenly  sprouting  a humming
forest of  hair-fine glass spines. The spines split, bisected,  split again,
exponential growth under the dome of the Tessier-Ashpool ice.
     The  roof  of  his mouth cleaved  painlessly,  admitting rootlets  that
whipped around his tongue, hungry for the taste of blue, to feed the crystal
forests of his eyes, forests that  pressed against  the green dome,  pressed
and  were  hindered, and spread, growing down, filling  the universe of T-A,
down  into the  waiting, hapless suburbs  of the city that  was  the mind of
TessierAshpool S.A.
     And  he was remembering an  ancient  story, a king placing  coins on  a
chessboard, doubling the amount at each square. . .
     Exponential. . .
     Darkness fell in from  every side, a  sphere of singing black, pressure
on the extended crystal nerves of the universe of data he had nearly become.
. .                                                                         
                                                                            
     And  when he was nothing,  compressed  at  the heart  of all that dark,
there came a point where the dark could be no more, and something tore.
)";

uint8_t enc_text[1024 * 16];
uint8_t dec_text[1024 * 16];

int main(const int argc, const char **args) {

  size_t written = 0;
  encode(src_text, enc_text, sizeof(src_text), sizeof(enc_text), &written);

  printf("%d comp vs %d plain\n", int(written), int(sizeof(src_text)));

//  decode(enc_text, dec_text, written, 1024 * 16, &written);

  uint8_t *data = (uint8_t*)decode(enc_text, written, &written);

  const int res = memcmp(src_text, dec_text, sizeof(src_text));
  assert(res == 0);

  getchar();

  return 0;
}
