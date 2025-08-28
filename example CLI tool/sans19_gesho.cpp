// Get Expected haSH Outputs
#include <iostream>
#include <cstring>
#include "sans19hash.h"
int main() {
    const char* test_strings[] = {
        "",
        "sans19",
        "Can y'all stop hating phonk? Seriously, half the time people trash the genre without even understanding what it is. They hear a distorted bassline or a gritty Memphis sample and immediately write it off as noise. But phonk isn't just a style—it's a vibe, a whole culture rooted in underground hip-hop, street racing, and raw emotion. It's the soundtrack of rebellion, of late nights drifting through neon-lit streets, of headphones blasting while the world fades away. From classic phonk with its lo-fi, eerie aesthetic to drift phonk with its aggressive energy, there's variety and depth if you take a second to actually listen. Artists pour their soul into this music, blending nostalgia, grit, and adrenaline into something that's more than just sound—it's atmosphere. Dismissing it just because it doesn't sound like chart-toppers or your favorite genre is lazy. Music is meant to challenge, to evoke, to make you feel something real, even if it's dark or chaotic. And phonk does exactly that. So before you keep riding the hate train, maybe dive a little deeper into the scene. Watch a drift montage synced to KSLV Noh or DVRST and feel the momentum. Because let's be real—you can't hate something you don't know.",
        "YOUR TAKING TOO LONG",
        "YOUR         LONG",
        "your taking too long :)",
        "your taking too long :) IS TAKING TOO LONG",
        "YOUR TOO BRIGHT",
        "YOUR TAKING TOO TOO",
        "YOUR         TOO TOO"
    };

    for (const char* s : test_strings) {
        Sans19Hash h;
        h.update(reinterpret_cast<const uint8_t*>(s), strlen(s));
        std::string hex = h.hexdigest();
        std::cout << hex << std::endl;
    }

    return 0;
}
