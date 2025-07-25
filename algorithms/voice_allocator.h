// Copyright 2012 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Polyphonic voice allocator.

#ifndef STMLIB_ALGORITHMS_VOICE_ALLOCATOR_H_
#define STMLIB_ALGORITHMS_VOICE_ALLOCATOR_H_

#include "stmlib/stmlib.h"
#include <cstring>

namespace stmlib {

enum VoiceAllocatorFlags {
  NOT_ALLOCATED = 0xff,
  ACTIVE_NOTE = 0x80
};

template<uint8_t capacity>
class VoiceAllocator {
 public: 
  VoiceAllocator() { }

  void Init() {
    size_ = 0;
    Reset();
  }

  uint8_t NoteOn(uint8_t note, uint8_t stealable_voice_index) {
    if (size_ == 0) {
      return NOT_ALLOCATED;
    }

    // First, check if there is a voice whose latest note was this note. In this
    // case, this voice will be responsible for retriggering this note.
    // Hint: if you're more into string instruments than keyboard instruments,
    // you can safely comment those lines.
    uint8_t voice = Find(note);

    // Then, try to find the least recently touched, currently inactive voice.
    if (voice == NOT_ALLOCATED) {
      for (uint8_t i = 0; i < capacity; ++i) {
        uint8_t maybe_voice = voice_for_touch_order_[i];
        bool active = latest_note_for_voice_[maybe_voice] & ACTIVE_NOTE;
        if (maybe_voice < size_ && !active) voice = maybe_voice;
      }
    }

    // If all voices are active, try to resort to stealing
    if (voice == NOT_ALLOCATED) voice = stealable_voice_index;
    if (voice == NOT_ALLOCATED) return voice; // Cannot steal; NOOP

    latest_note_for_voice_[voice] = note | ACTIVE_NOTE;
    Touch(voice);
    return voice;
  }

  uint8_t NoteOff(uint8_t note) {
    uint8_t voice = Find(note);
    if (voice != NOT_ALLOCATED) {
      latest_note_for_voice_[voice] &= 0x7f; // ACTIVE_NOTE = false
      Touch(voice);
    }
    return voice;
  }

  // Find the voice whose latest note was this note
  uint8_t Find(uint8_t note) const {
    for (uint8_t i = 0; i < size_; ++i) {
      if ((latest_note_for_voice_[i] & 0x7f) == note) {
        return i;
      }
    }
    return NOT_ALLOCATED;
  }

  void Reset() {
    memset(&latest_note_for_voice_, 0, sizeof(latest_note_for_voice_));
    for (uint8_t i = 0; i < capacity; ++i) {
      voice_for_touch_order_[i] = capacity - i - 1;
    }
  }

  inline void AllNotesOff() {
    for (uint8_t i = 0; i < capacity; ++i) {
      latest_note_for_voice_[i] &= 0x7f; // ACTIVE_NOTE = false
    }
  }

  inline void set_size(uint8_t size) {
    size_ = size;
  }

  inline uint8_t size() const { return size_; }

 private:
  void Touch(uint8_t voice) {
    int8_t source = capacity - 1;
    int8_t destination = capacity - 1;
    while (source >= 0) {
      if (voice_for_touch_order_[source] != voice) {
        voice_for_touch_order_[destination--] = voice_for_touch_order_[source];
      }
      --source;
    }
    voice_for_touch_order_[0] = voice;
  }
   
  // Maps voice index to its most recent note (even after NoteOff!)
  uint8_t latest_note_for_voice_[capacity];

  // Holds the indices of the voices sorted by most recent usage first
  uint8_t voice_for_touch_order_[capacity];

  // Number of available voices (may be less than static capacity)
  uint8_t size_;

  DISALLOW_COPY_AND_ASSIGN(VoiceAllocator);
};

}  // namespace stmlib

#endif  // STMLIB_ALGORITHMS_VOICE_ALLOCATOR_H_
