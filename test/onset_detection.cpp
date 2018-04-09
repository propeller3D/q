#include <q/literals.hpp>
#include <q/sfx.hpp>
#include <q_io/audio_file.hpp>
#include <vector>
#include <string>

namespace q = cycfi::q;
namespace audio_file = q::audio_file;
using namespace q::literals;

void process(std::string name)
{
   ////////////////////////////////////////////////////////////////////////////
   // Read audio file

   auto src = audio_file::reader{"audio_files/" + name + ".aif"};
   std::uint32_t const sps = src.sps();

   std::vector<float> in(src.length());
   src.read(in);

   auto max_val = *std::max_element(in.begin(), in.end(),
      [](auto a, auto b) { return std::abs(a) < std::abs(b); }
   );

   ////////////////////////////////////////////////////////////////////////////
   // Onset detection

   constexpr auto n_channels = 4;

   std::vector<float> out(src.length() * n_channels);
   auto i = out.begin();

   q::dc_block dc_blk{ 1_Hz, sps };
   q::onset onset{ 0.8f, 0.05, 50_ms, 100_ms, sps };
   q::peak_envelope_follower env{ 100_ms, sps };

   for (auto s : in)
   {
      // Normalize
      s *= 1.0 / max_val;

      // Original
      *i++ = s;

      // Onset
      auto o = onset(s, env(std::abs(s)));
      *i++ = o * 0.8;

      // The envelope
      *i++ = env();

      // Lowpassed envelope
      *i++ = onset._lp();
   }

   ////////////////////////////////////////////////////////////////////////////
   // Write to a wav file

   auto wav = audio_file::writer{
      "results/onset_" + name + ".wav", audio_file::wav, audio_file::_16_bits
    , n_channels, sps
   };
   wav.write(out);
}

int main()
{
   process("sin_440");
   process("1-Low E");
   process("Tapping D");
   process("Hammer-Pull High E");
   return 0;
}