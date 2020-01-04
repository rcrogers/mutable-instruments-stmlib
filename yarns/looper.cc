// Copyright 2019 Chris Rogers.
//
// Author: Chris Rogers (teukros@gmail.com)
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
// Looper.

#include "yarns/looper.h"

#include "yarns/part.h"

namespace yarns {

namespace looper {

void Tape::RemoveAll() {
  std::fill(
    &notes_[0],
    &notes_[kMaxNotes],
    Note()
  );
  head_link_.on_index = kNullIndex;
  head_link_.off_index = kNullIndex;
  oldest_index_ = 0;
  newest_index_ = 0;
}

void Tape::ResetHead() {
  uint8_t next_index;

  while (true) {
    next_index = PeekNextOn();
    if (
      next_index == kNullIndex ||
      notes_[head_link_.on_index].on_pos >= notes_[next_index].on_pos
    ) {
      break;
    }
    head_link_.on_index = next_index;
  }

  while (true) {
    next_index = PeekNextOff();
    if (
      next_index == kNullIndex ||
      notes_[head_link_.off_index].off_pos >= notes_[next_index].off_pos
    ) {
      break;
    }
    head_link_.off_index = next_index;
  }
}

void Tape::RemoveOldestNote(Part* part, uint16_t current_pos) {
  RemoveNote(part, current_pos, oldest_index_);
  if (!IsEmpty()) {
    oldest_index_ = stmlib::modulo(oldest_index_ + 1, kMaxNotes);
  }
}

void Tape::RemoveNewestNote(Part* part, uint16_t current_pos) {
  RemoveNote(part, current_pos, newest_index_);
  if (!IsEmpty()) {
    newest_index_ = stmlib::modulo(newest_index_ - 1, kMaxNotes);
  }
}

uint8_t Tape::PeekNextOn() {
  if (head_link_.on_index == kNullIndex) {
    return kNullIndex;
  }
  return notes_[head_link_.on_index].next_link.on_index;
}

uint8_t Tape::PeekNextOff() {
  if (head_link_.off_index == kNullIndex) {
    return kNullIndex;
  }
  return notes_[head_link_.off_index].next_link.off_index;
}

void Tape::Advance(Part* part, bool play, uint16_t old_pos, uint16_t new_pos) {
  uint8_t seen_index;
  uint8_t next_index;

  seen_index = looper::kNullIndex;
  while (true) {
    next_index = PeekNextOff();
    if (next_index == kNullIndex || next_index == seen_index) {
      break;
    }
    if (seen_index == kNullIndex) {
      seen_index = next_index;
    }
    const Note& next_note = notes_[next_index];
    if (!Passed(next_note.off_pos, old_pos, new_pos)) {
      break;
    }
    head_link_.off_index = next_index;

    if (play) {
      part->InternalNoteOff(next_note.pitch);
    }
  }

  seen_index = looper::kNullIndex;
  while (true) {
    next_index = PeekNextOn();
    if (next_index == kNullIndex || next_index == seen_index) {
      break;
    }
    if (seen_index == kNullIndex) {
      seen_index = next_index;
    }
    const Note& next_note = notes_[next_index];
    if (!Passed(next_note.on_pos, old_pos, new_pos)) {
      break;
    }
    head_link_.on_index = next_index;

    if (next_note.next_link.off_index == kNullIndex) {
      // If the next 'on' note doesn't yet have an off_index, it's still held
      // and has been for an entire loop -- instead of redundantly turning the
      // note on, set an off_pos
      InsertOff(next_note.on_pos, next_index);
      continue;
    }

    if (play) {
      part->InternalNoteOn(next_note.pitch, next_note.velocity);
    }
  }
}

uint8_t Tape::RecordNoteOn(Part* part, uint16_t pos, uint8_t pitch, uint8_t velocity) {
  if (!IsEmpty()) {
    newest_index_ = stmlib::modulo(1 + newest_index_, kMaxNotes);
  }
  if (newest_index_ == oldest_index_) {
    RemoveOldestNote(part, pos);
  }

  InsertOn(pos, newest_index_);

  Note& note = notes_[newest_index_];
  note.pitch = pitch;
  note.velocity = velocity;
  note.off_pos = 0;
  note.next_link.off_index = kNullIndex;

  return newest_index_;
}

// Returns whether the NoteOff should be sent
bool Tape::RecordNoteOff(uint16_t pos, uint8_t index) {
  if (notes_[index].next_link.off_index != kNullIndex) {
    // off_pos was already set by Advance, so the note was held for an entire
    // loop, so the note should play continuously and not be turned off now
    return false;
  }
  InsertOff(pos, index);
  return true;
}

bool Tape::Passed(uint16_t target, uint16_t before, uint16_t after) {
  if (before < after) {
    return (target > before and target <= after);
  } else {
    return (target > before or target <= after);
  }
}

void Tape::InsertOn(uint16_t pos, uint8_t index) {
  Note& note = notes_[index];
  note.on_pos = pos;
  if (head_link_.on_index == kNullIndex) {
    // there is no prev note to link to this one, so link it to itself
    note.next_link.on_index = index;
  } else {
    Note& head_note = notes_[head_link_.on_index];
    note.next_link.on_index = head_note.next_link.on_index;
    head_note.next_link.on_index = index;
  }
  head_link_.on_index = index;
}

void Tape::InsertOff(uint16_t pos, uint8_t index) {
  Note& note = notes_[index];
  note.off_pos = pos;
  if (head_link_.off_index == kNullIndex) {
    // there is no prev note to link to this one, so link it to itself
    note.next_link.off_index = index;
  } else {
    Note& head_note = notes_[head_link_.off_index];
    note.next_link.off_index = head_note.next_link.off_index;
    head_note.next_link.off_index = index;
  }
  head_link_.off_index = index;
}

void Tape::RemoveNote(Part* part, uint16_t current_pos, uint8_t target_index) {
  if (IsEmpty()) {
    return;
  }

  Note& target_note = notes_[target_index];
  bool target_has_off = (target_note.next_link.off_index != kNullIndex);
  uint8_t search_prev_index;
  uint8_t search_next_index;

  if (target_has_off && Passed(current_pos, target_note.on_pos, target_note.off_pos)) {
    // If this note was completely recorded and the looper is currently playing
    // the note, turn it off
    part->InternalNoteOff(target_note.pitch);
  }

  // general concern -- next index never matches target
  // bc next index points into a separate cycle of notes
  // most likely that cycle would be a single uninitialized note, e.g. if
  // target_note has kNullIndex for its next index

  search_prev_index = target_index;
  while (true) {
    search_next_index = notes_[search_prev_index].next_link.on_index;
    if (search_next_index == target_index) {
      break;
    }
    search_prev_index = search_next_index;
  }
  notes_[search_prev_index].next_link.on_index = target_note.next_link.on_index;
  target_note.next_link.on_index = kNullIndex; // unneeded?
  if (target_index == search_prev_index) {
    // If this was the last note
    head_link_.on_index = kNullIndex;
  } else if (target_index == head_link_.on_index) {
    head_link_.on_index = search_prev_index;
  }

  if (!target_has_off) {
    // Don't try to relink off_index
    return;
  }

  search_prev_index = target_index;
  while (true) {
    search_next_index = notes_[search_prev_index].next_link.off_index;
    if (search_next_index == target_index) {
      break;
    }
    search_prev_index = search_next_index;
  }
  notes_[search_prev_index].next_link.off_index = target_note.next_link.off_index;
  target_note.next_link.off_index = kNullIndex;
  if (target_index == search_prev_index) {
    // If this was the last note
    head_link_.off_index = kNullIndex;
  } else if (target_index == head_link_.off_index) {
    head_link_.off_index = search_prev_index;
  }
}

} // namespace looper
}  // namespace yarns