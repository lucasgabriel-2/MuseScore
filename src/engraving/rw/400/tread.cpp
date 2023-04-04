/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "tread.h"

#include "../../types/typesconv.h"
#include "../../types/symnames.h"

#include "../../libmscore/score.h"
#include "../../libmscore/factory.h"

#include "../../libmscore/tempotext.h"
#include "../../libmscore/stafftext.h"
#include "../../libmscore/stafftextbase.h"
#include "../../libmscore/dynamic.h"
#include "../../libmscore/harmony.h"
#include "../../libmscore/chordlist.h"
#include "../../libmscore/fret.h"
#include "../../libmscore/tremolobar.h"
#include "../../libmscore/sticking.h"
#include "../../libmscore/systemtext.h"
#include "../../libmscore/playtechannotation.h"
#include "../../libmscore/rehearsalmark.h"
#include "../../libmscore/instrchange.h"
#include "../../libmscore/staffstate.h"
#include "../../libmscore/figuredbass.h"
#include "../../libmscore/part.h"
#include "../../libmscore/fermata.h"
#include "../../libmscore/image.h"
#include "../../libmscore/tuplet.h"
#include "../../libmscore/text.h"
#include "../../libmscore/beam.h"
#include "../../libmscore/ambitus.h"
#include "../../libmscore/accidental.h"
#include "../../libmscore/marker.h"
#include "../../libmscore/jump.h"
#include "../../libmscore/measurenumber.h"
#include "../../libmscore/mmrestrange.h"
#include "../../libmscore/systemdivider.h"
#include "../../libmscore/actionicon.h"
#include "../../libmscore/arpeggio.h"
#include "../../libmscore/articulation.h"
#include "../../libmscore/audio.h"
#include "../../libmscore/bagpembell.h"
#include "../../libmscore/barline.h"
#include "../../libmscore/chord.h"
#include "../../libmscore/bend.h"
#include "../../libmscore/box.h"
#include "../../libmscore/textframe.h"
#include "../../libmscore/layoutbreak.h"
#include "../../libmscore/stafftypechange.h"
#include "../../libmscore/bracket.h"
#include "../../libmscore/breath.h"
#include "../../libmscore/note.h"
#include "../../libmscore/spanner.h"
#include "../../libmscore/fingering.h"
#include "../../libmscore/notedot.h"
#include "../../libmscore/chordline.h"
#include "../../libmscore/timesig.h"
#include "../../libmscore/lyrics.h"
#include "../../libmscore/stem.h"
#include "../../libmscore/stemslash.h"
#include "../../libmscore/hook.h"
#include "../../libmscore/tremolo.h"
#include "../../libmscore/clef.h"
#include "../../libmscore/glissando.h"
#include "../../libmscore/gradualtempochange.h"
#include "../../libmscore/line.h"
#include "../../libmscore/textlinebase.h"
#include "../../libmscore/groups.h"
#include "../../libmscore/hairpin.h"
#include "../../libmscore/keysig.h"

#include "../xmlreader.h"
#include "../206/read206.h"

#include "readcontext.h"
#include "textbaserw.h"
#include "propertyrw.h"
#include "engravingitemrw.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

template<typename T>
static bool try_read(EngravingItem* el, XmlReader& xml, ReadContext& ctx)
{
    T* t = dynamic_cast<T*>(el);
    if (!t) {
        return false;
    }
    TRead::read(t, xml, ctx);
    return true;
}

void TRead::readItem(EngravingItem* el, XmlReader& xml, ReadContext& ctx)
{
    if (try_read<Sticking>(el, xml, ctx)) {
    } else if (try_read<SystemText>(el, xml, ctx)) {
    } else if (try_read<PlayTechAnnotation>(el, xml, ctx)) {
    } else if (try_read<RehearsalMark>(el, xml, ctx)) {
    } else if (try_read<InstrumentChange>(el, xml, ctx)) {
    } else if (try_read<StaffState>(el, xml, ctx)) {
    } else if (try_read<FiguredBass>(el, xml, ctx)) {
    } else if (try_read<Marker>(el, xml, ctx)) {
    } else if (try_read<Jump>(el, xml, ctx)) {
    } else if (try_read<HBox>(el, xml, ctx)) {
    } else if (try_read<VBox>(el, xml, ctx)) {
    } else if (try_read<TBox>(el, xml, ctx)) {
    } else if (try_read<FBox>(el, xml, ctx)) {
    } else {
        UNREACHABLE;
    }
}

void TRead::read(TextBase* t, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        if (!TextBaseRW::readProperties(t, xml, ctx)) {
            xml.unknown();
        }
    }
}

void TRead::read(TempoText* t, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "tempo") {
            t->setTempo(TConv::fromXml(e.readAsciiText(), Constants::defaultTempo));
        } else if (tag == "followText") {
            t->setFollowText(e.readInt());
        } else if (!TextBaseRW::readProperties(t, e, ctx)) {
            e.unknown();
        }
    }
    // check sanity
    if (t->xmlText().isEmpty()) {
        t->setXmlText(String(u"<sym>metNoteQuarterUp</sym> = %1").arg(int(lrint(t->tempo().toBPM().val))));
        t->setVisible(false);
    }
}

void TRead::read(StaffText* t, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<StaffTextBase*>(t), xml, ctx);
}

void TRead::read(StaffTextBase* t, XmlReader& xml, ReadContext& ctx)
{
    t->clear();

    while (xml.readNextStartElement()) {
        if (!readProperties(t, xml, ctx)) {
            xml.unknown();
        }
    }
}

bool TRead::readProperties(StaffTextBase* t, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "MidiAction") {
        int channel = e.intAttribute("channel", 0);
        String name = e.attribute("name");
        bool found = false;
        size_t n = t->channelActions().size();
        for (size_t i = 0; i < n; ++i) {
            ChannelActions* a = &t->channelActions()[i];
            if (a->channel == channel) {
                a->midiActionNames.append(name);
                found = true;
                break;
            }
        }
        if (!found) {
            ChannelActions a;
            a.channel = channel;
            a.midiActionNames.append(name);
            t->channelActions().push_back(a);
        }
        e.readNext();
    } else if (tag == "channelSwitch" || tag == "articulationChange") {
        voice_idx_t voice = static_cast<voice_idx_t>(e.intAttribute("voice", -1));
        if (voice < VOICES) {
            t->setChannelName(voice, e.attribute("name"));
        } else if (voice == mu::nidx) {
            // no voice applies channel to all voices for
            // compatibility
            for (voice_idx_t i = 0; i < VOICES; ++i) {
                t->setChannelName(i, e.attribute("name"));
            }
        }
        e.readNext();
    } else if (tag == "aeolus") {
        int group = e.intAttribute("group", -1);
        if (group >= 0 && group < 4) {
            t->setAeolusStop(group, e.readInt());
        } else {
            e.readNext();
        }
        t->setSetAeolusStops(true);
    } else if (tag == "swing") {
        DurationType swingUnit = TConv::fromXml(e.asciiAttribute("unit"), DurationType::V_INVALID);
        int unit = 0;
        if (swingUnit == DurationType::V_EIGHTH) {
            unit = Constants::division / 2;
        } else if (swingUnit == DurationType::V_16TH) {
            unit = Constants::division / 4;
        } else if (swingUnit == DurationType::V_ZERO) {
            unit = 0;
        }
        int ratio = e.intAttribute("ratio", 60);
        t->setSwing(true);
        t->setSwingParameters(unit, ratio);
        e.readNext();
    } else if (tag == "capo") {
        int fretId = e.intAttribute("fretId", 0);
        t->setCapo(fretId);
        e.readNext();
    } else if (!TextBaseRW::readProperties(t, e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(Dynamic* d, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag = e.name();
        if (tag == "subtype") {
            d->setDynamicType(e.readText());
        } else if (tag == "velocity") {
            d->setVelocity(e.readInt());
        } else if (tag == "dynType") {
            d->setDynRange(TConv::fromXml(e.readAsciiText(), DynamicRange::STAFF));
        } else if (tag == "veloChange") {
            d->setChangeInVelocity(e.readInt());
        } else if (tag == "veloChangeSpeed") {
            d->setVelChangeSpeed(TConv::fromXml(e.readAsciiText(), DynamicSpeed::NORMAL));
        } else if (!TextBaseRW::readProperties(d, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Harmony* h, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "base") {
            h->setBaseTpc(e.readInt());
        } else if (tag == "baseCase") {
            h->setBaseCase(static_cast<NoteCaseType>(e.readInt()));
        } else if (tag == "extension") {
            h->setId(e.readInt());
        } else if (tag == "name") {
            h->setTextName(e.readText());
        } else if (tag == "root") {
            h->setRootTpc(e.readInt());
        } else if (tag == "rootCase") {
            h->setRootCase(static_cast<NoteCaseType>(e.readInt()));
        } else if (tag == "function") {
            h->setFunction(e.readText());
        } else if (tag == "degree") {
            int degreeValue = 0;
            int degreeAlter = 0;
            String degreeType;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "degree-value") {
                    degreeValue = e.readInt();
                } else if (t == "degree-alter") {
                    degreeAlter = e.readInt();
                } else if (t == "degree-type") {
                    degreeType = e.readText();
                } else {
                    e.unknown();
                }
            }
            if (degreeValue <= 0 || degreeValue > 13
                || degreeAlter < -2 || degreeAlter > 2
                || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                LOGD("incorrect degree: degreeValue=%d degreeAlter=%d degreeType=%s",
                     degreeValue, degreeAlter, muPrintable(degreeType));
            } else {
                if (degreeType == "add") {
                    h->addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ADD));
                } else if (degreeType == "alter") {
                    h->addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ALTER));
                } else if (degreeType == "subtract") {
                    h->addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT));
                }
            }
        } else if (tag == "leftParen") {
            h->setLeftParen(true);
            e.readNext();
        } else if (tag == "rightParen") {
            h->setRightParen(true);
            e.readNext();
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::POS_ABOVE)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::HARMONY_TYPE)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::PLAY)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::HARMONY_VOICE_LITERAL)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::HARMONY_VOICING)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::HARMONY_DURATION)) {
        } else if (!TextBaseRW::readProperties(h, e, ctx)) {
            e.unknown();
        }
    }

    h->afterRead();
}

void TRead::read(FretDiagram* d, XmlReader& e, ReadContext& ctx)
{
    // Read the old format first
    bool hasBarre = false;
    bool haveReadNew = false;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        // Check for new format fret diagram
        if (haveReadNew) {
            e.skipCurrentElement();
            continue;
        }
        if (tag == "fretDiagram") {
            // Read new
            while (e.readNextStartElement()) {
                const AsciiStringView tag(e.name());

                if (tag == "string") {
                    int no = e.intAttribute("no");
                    while (e.readNextStartElement()) {
                        const AsciiStringView t(e.name());
                        if (t == "dot") {
                            int fret = e.intAttribute("fret", 0);
                            FretDotType dtype = FretItem::nameToDotType(e.readText());
                            d->setDot(no, fret, true, dtype);
                        } else if (t == "marker") {
                            FretMarkerType mtype = FretItem::nameToMarkerType(e.readText());
                            d->setMarker(no, mtype);
                        } else if (t == "fingering") {
                            e.readText();
                            /*setFingering(no, e.readInt()); NOTE:JT todo */
                        } else {
                            e.unknown();
                        }
                    }
                } else if (tag == "barre") {
                    int start = e.intAttribute("start", -1);
                    int end = e.intAttribute("end", -1);
                    int fret = e.readInt();

                    d->setBarre(start, end, fret);
                } else if (!EngravingItemRW::readProperties(d, e, ctx)) {
                    e.unknown();
                }
            }
            haveReadNew = true;
        }
        // Check for new properties
        else if (tag == "showNut") {
            PropertyRW::readProperty(d, e, ctx, Pid::FRET_NUT);
        } else if (tag == "orientation") {
            PropertyRW::readProperty(d, e, ctx, Pid::ORIENTATION);
        }
        // Then read the rest if there is no new format diagram (compatibility read)
        else if (tag == "strings") {
            PropertyRW::readProperty(d, e, ctx, Pid::FRET_STRINGS);
        } else if (tag == "frets") {
            PropertyRW::readProperty(d, e, ctx, Pid::FRET_FRETS);
        } else if (tag == "fretOffset") {
            PropertyRW::readProperty(d, e, ctx, Pid::FRET_OFFSET);
        } else if (tag == "string") {
            int no = e.intAttribute("no");
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "dot") {
                    d->setDot(no, e.readInt());
                } else if (t == "marker") {
                    d->setMarker(no, Char(e.readInt()) == u'X' ? FretMarkerType::CROSS : FretMarkerType::CIRCLE);
                }
                /*else if (t == "fingering")
                      setFingering(no, e.readInt());*/
                else {
                    e.unknown();
                }
            }
        } else if (tag == "barre") {
            hasBarre = e.readBool();
        } else if (tag == "mag") {
            PropertyRW::readProperty(d, e, ctx, Pid::MAG);
        } else if (tag == "Harmony") {
            Harmony* h = new Harmony(d->score()->dummy()->segment());
            read(h, e, ctx);
            d->add(h);
        } else if (!EngravingItemRW::readProperties(d, e, ctx)) {
            e.unknown();
        }
    }

    // Old handling of barres
    if (hasBarre) {
        for (int s = 0; s < d->strings(); ++s) {
            for (auto& dot : d->dot(s)) {
                if (dot.exists()) {
                    d->setBarre(s, -1, dot.fret);
                    return;
                }
            }
        }
    }
}

void TRead::read(TremoloBar* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        auto tag = e.name();
        if (tag == "point") {
            PitchValue pv;
            pv.time    = e.intAttribute("time");
            pv.pitch   = e.intAttribute("pitch");
            pv.vibrato = e.intAttribute("vibrato");
            b->points().push_back(pv);
            e.readNext();
        } else if (tag == "mag") {
            b->setUserMag(e.readDouble(0.1, 10.0));
        } else if (PropertyRW::readStyledProperty(b, tag, e, ctx)) {
        } else if (tag == "play") {
            b->setPlay(e.readInt());
        } else if (PropertyRW::readProperty(b, tag, e, ctx, Pid::LINE_WIDTH)) {
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Sticking* s, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<TextBase*>(s), xml, ctx);
}

void TRead::read(SystemText* t, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<StaffTextBase*>(t), xml, ctx);
}

void TRead::read(PlayTechAnnotation* a, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        const AsciiStringView tag(xml.name());

        if (PropertyRW::readProperty(a, tag, xml, ctx, Pid::PLAY_TECH_TYPE)) {
            continue;
        }

        if (!readProperties(static_cast<StaffTextBase*>(a), xml, ctx)) {
            xml.unknown();
        }
    }
}

void TRead::read(RehearsalMark* m, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<TextBase*>(m), xml, ctx);
}

void TRead::read(InstrumentChange* c, XmlReader& e, ReadContext& ctx)
{
    Instrument inst;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Instrument") {
            inst.read(e, c->part());
        } else if (tag == "init") {
            c->setInit(e.readBool());
        } else if (!TextBaseRW::readProperties(c, e, ctx)) {
            e.unknown();
        }
    }

    if (c->score()->mscVersion() < 206) {
        // previous versions did not honor transposition of instrument change
        // except in ways that it should not have
        // notes entered before the instrument change was added would not be altered,
        // so original transposition remained in effect
        // notes added afterwards would be transposed by both intervals, resulting in tpc corruption
        // here we set the instrument change to inherit the staff transposition to emulate previous versions
        // in Note::read(), we attempt to fix the tpc corruption
        // There is also code in read206 to try to deal with this, but it is out of date and therefore disabled
        // What this means is, scores created in 2.1 or later should be fine, scores created in 2.0 maybe not so much

        Interval v = c->staff() ? c->staff()->part()->instrument(c->tick())->transpose() : 0;
        inst.setTranspose(v);
    }

    c->setInstrument(inst);
}

//---------------------------------------------------------
//    for import of 1.3 scores
//---------------------------------------------------------

static SymId convertFromOldId(int val)
{
    SymId symId = SymId::noSym;
    switch (val) {
    case 32: symId = SymId::accidentalSharp;
        break;
    case 33: symId = SymId::accidentalThreeQuarterTonesSharpArrowUp;
        break;
    case 34: symId = SymId::accidentalQuarterToneSharpArrowDown;
        break;
    // case 35: // "sharp arrow both" missing in SMuFL
    case 36: symId = SymId::accidentalQuarterToneSharpStein;
        break;
    case 37: symId = SymId::accidentalBuyukMucennebSharp;
        break;
    case 38: symId = SymId::accidentalKomaSharp;
        break;
    case 39: symId = SymId::accidentalThreeQuarterTonesSharpStein;
        break;
    case 40: symId = SymId::accidentalNatural;
        break;
    case 41: symId = SymId::accidentalQuarterToneSharpNaturalArrowUp;
        break;
    case 42: symId = SymId::accidentalQuarterToneFlatNaturalArrowDown;
        break;
    // case 43: // "natural arrow both" missing in SMuFL
    case 44: symId = SymId::accidentalFlat;
        break;
    case 45: symId = SymId::accidentalQuarterToneFlatArrowUp;
        break;
    case 46: symId = SymId::accidentalThreeQuarterTonesFlatArrowDown;
        break;
    // case 47: // "flat arrow both" missing in SMuFL
    case 48: symId = SymId::accidentalBakiyeFlat;
        break;
    case 49: symId = SymId::accidentalBuyukMucennebFlat;
        break;
    case 50: symId = SymId::accidentalThreeQuarterTonesFlatZimmermann;
        break;
    case 51: symId = SymId::accidentalQuarterToneFlatStein;
        break;
    // case 52: // "mirrored flat slash" missing in SMuFL
    case 53: symId = SymId::accidentalDoubleFlat;
        break;
    // case 54: // "flat flat slash" missing in SMuFL
    case 55: symId = SymId::accidentalDoubleSharp;
        break;
    case 56: symId = SymId::accidentalSori;
        break;
    case 57: symId = SymId::accidentalKoron;
        break;
    default:
        LOGD("MuseScore 1.3 symbol id corresponding to <%d> not found", val);
        symId = SymId::noSym;
        break;
    }
    return symId;
}

void TRead::read(KeySig* s, XmlReader& e, ReadContext& ctx)
{
    KeySigEvent sig;
    int subtype = 0;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "CustDef" || tag == "KeySym") {
            CustDef cd;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "sym") {
                    AsciiStringView val(e.readAsciiText());
                    bool valid;
                    SymId id = SymId(val.toInt(&valid));
                    if (!valid) {
                        id = SymNames::symIdByName(val);
                    }
                    if (s->score()->mscVersion() <= 114) {
                        if (valid) {
                            id = convertFromOldId(val.toInt(&valid));
                        }
                        if (!valid) {
                            id = SymNames::symIdByOldName(val);
                        }
                    }
                    cd.sym = id;
                } else if (t == "def") {
                    cd.degree = e.intAttribute("degree", 0);
                    cd.octAlt = e.intAttribute("octAlt", 0);
                    cd.xAlt = e.doubleAttribute("xAlt", 0.0);
                    e.readNext();
                } else if (t == "pos") { // for older files
                    double prevx = 0;
                    double accidentalGap = s->score()->styleS(Sid::keysigAccidentalDistance).val();
                    double _spatium = s->spatium();
                    // count default x position
                    for (CustDef& cd : sig.customKeyDefs()) {
                        prevx += s->symWidth(cd.sym) / _spatium + accidentalGap + cd.xAlt;
                    }
                    bool flat = std::string(SymNames::nameForSymId(cd.sym).ascii()).find("Flat") != std::string::npos;
                    // if x not there, use default step
                    cd.xAlt = e.doubleAttribute("x", prevx) - prevx;
                    // if y not there, use middle line
                    int line = static_cast<int>(e.doubleAttribute("y", 2) * 2);
                    cd.degree = (3 - line) % 7;
                    cd.degree += (cd.degree < 0) ? 7 : 0;
                    line += flat ? -1 : 1; // top point in treble for # is -1 (gis), and for b is 1 (es)
                    cd.octAlt = static_cast<int>((line - (line >= 0 ? 0 : 6)) / 7);
                    e.readNext();
                } else {
                    e.unknown();
                }
            }
            sig.customKeyDefs().push_back(cd);
        } else if (tag == "showCourtesySig") {
            s->setShowCourtesy(e.readInt());
        } else if (tag == "showNaturals") {           // obsolete
            e.readInt();
        } else if (tag == "accidental") {
            sig.setKey(Key(e.readInt()));
        } else if (tag == "natural") {                // obsolete
            e.readInt();
        } else if (tag == "custom") {
            e.readInt();
            sig.setCustom(true);
        } else if (tag == "mode") {
            String m(e.readText());
            if (m == "none") {
                sig.setMode(KeyMode::NONE);
            } else if (m == "major") {
                sig.setMode(KeyMode::MAJOR);
            } else if (m == "minor") {
                sig.setMode(KeyMode::MINOR);
            } else if (m == "dorian") {
                sig.setMode(KeyMode::DORIAN);
            } else if (m == "phrygian") {
                sig.setMode(KeyMode::PHRYGIAN);
            } else if (m == "lydian") {
                sig.setMode(KeyMode::LYDIAN);
            } else if (m == "mixolydian") {
                sig.setMode(KeyMode::MIXOLYDIAN);
            } else if (m == "aeolian") {
                sig.setMode(KeyMode::AEOLIAN);
            } else if (m == "ionian") {
                sig.setMode(KeyMode::IONIAN);
            } else if (m == "locrian") {
                sig.setMode(KeyMode::LOCRIAN);
            } else {
                sig.setMode(KeyMode::UNKNOWN);
            }
        } else if (tag == "subtype") {
            subtype = e.readInt();
        } else if (tag == "forInstrumentChange") {
            s->setForInstrumentChange(e.readBool());
        } else if (!EngravingItemRW::readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
    // for backward compatibility
    if (!sig.isValid()) {
        sig.initFromSubtype(subtype);
    }
    if (sig.custom() && sig.customKeyDefs().empty()) {
        sig.setMode(KeyMode::NONE);
    }

    s->setKeySigEvent(sig);
}

void TRead::read(StaffState* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            s->setStaffStateType(StaffStateType(e.readInt()));
        } else if (tag == "Instrument") {
            Instrument i;
            i.read(e, nullptr);
            s->setInstrument(std::move(i));
        } else if (!EngravingItemRW::readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(FiguredBass* b, XmlReader& e, ReadContext& ctx)
{
    String normalizedText;
    int idx = 0;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "ticks") {
            b->setTicks(e.readFraction());
        } else if (tag == "onNote") {
            b->setOnNote(e.readInt() != 0l);
        } else if (tag == "FiguredBassItem") {
            FiguredBassItem* pItem = b->createItem(idx++);
            pItem->setTrack(b->track());
            pItem->setParent(b);
            pItem->read(e);
            b->appendItem(pItem);
            // add item normalized text
            if (!normalizedText.isEmpty()) {
                normalizedText.append('\n');
            }
            normalizedText.append(pItem->normalizedText());
        }
//            else if (tag == "style")
//                  setStyledPropertyListIdx(e.readElementText());
        else if (!TextBaseRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
    // if items could be parsed set normalized text
    if (b->items().size() > 0) {
        b->setXmlText(normalizedText);          // this is the text to show while editing
    }
}

void TRead::read(Fermata* f, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(f, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Fermata* f, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "subtype") {
        AsciiStringView s = e.readAsciiText();
        SymId id = SymNames::symIdByName(s);
        f->setSymId(id);
    } else if (tag == "play") {
        f->setPlay(e.readBool());
    } else if (tag == "timeStretch") {
        f->setTimeStretch(e.readDouble());
    } else if (tag == "offset") {
        if (f->score()->mscVersion() > 114) {
            EngravingItemRW::readProperties(f, e, ctx);
        } else {
            e.skipCurrentElement();       // ignore manual layout in older scores
        }
    } else if (EngravingItemRW::readProperties(f, e, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(Image* img, XmlReader& e, ReadContext& ctx)
{
    //! TODO Should be replaced with `ctx.mscVersion()`
    //! But at the moment, `ctx` is not set everywhere
    int mscVersion = img->score()->mscVersion();
    if (mscVersion <= 114) {
        img->setSizeIsSpatium(false);
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "autoScale") {
            PropertyRW::readProperty(img, e, ctx, Pid::AUTOSCALE);
        } else if (tag == "size") {
            PropertyRW::readProperty(img, e, ctx, Pid::SIZE);
        } else if (tag == "lockAspectRatio") {
            PropertyRW::readProperty(img, e, ctx, Pid::LOCK_ASPECT_RATIO);
        } else if (tag == "sizeIsSpatium") {
            // setting this using the property Pid::SIZE_IS_SPATIUM breaks, because the
            // property setter attempts to maintain a constant size. If we're reading, we
            // don't want to do that, because the stored size will be in:
            //    mm if size isn't spatium
            //    sp if size is spatium
            img->setSizeIsSpatium(e.readBool());
        } else if (tag == "path") {
            img->setStorePath(e.readText());
        } else if (tag == "linkPath") {
            img->setLinkPath(e.readText());
        } else if (tag == "subtype") {    // obsolete
            e.skipCurrentElement();
        } else if (!TRead::readProperties(img, e, ctx)) {
            e.unknown();
        }
    }

    img->load();
}

void TRead::read(Tuplet* t, XmlReader& e, ReadContext& ctx)
{
    t->setId(e.intAttribute("id", 0));

    Text* number = nullptr;
    Fraction ratio;
    TDuration baseLen;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (PropertyRW::readStyledProperty(t, tag, e, ctx)) {
        } else if (tag == "bold") { //important that these properties are read after number is created
            bool val = e.readInt();
            if (number) {
                number->setBold(val);
            }
            if (t->isStyled(Pid::FONT_STYLE)) {
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
        } else if (tag == "italic") {
            bool val = e.readInt();
            if (number) {
                number->setItalic(val);
            }
            if (t->isStyled(Pid::FONT_STYLE)) {
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
        } else if (tag == "underline") {
            bool val = e.readInt();
            if (number) {
                number->setUnderline(val);
            }
            if (t->isStyled(Pid::FONT_STYLE)) {
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
        } else if (tag == "strike") {
            bool val = e.readInt();
            if (number) {
                number->setStrike(val);
            }
            if (t->isStyled(Pid::FONT_STYLE)) {
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
        } else if (tag == "normalNotes") {
            ratio.setDenominator(e.readInt());
        } else if (tag == "actualNotes") {
            ratio.setNumerator(e.readInt());
        } else if (tag == "p1") {
            t->setUserPoint1(e.readPoint() * t->score()->spatium());
        } else if (tag == "p2") {
            t->setUserPoint2(e.readPoint() * t->score()->spatium());
        } else if (tag == "baseNote") {
            baseLen = TDuration(TConv::fromXml(e.readAsciiText(), DurationType::V_INVALID));
        } else if (tag == "baseDots") {
            baseLen.setDots(e.readInt());
        } else if (tag == "Number") {
            number = Factory::createText(t, TextStyleType::TUPLET);
            number->setComposition(true);
            number->setParent(t);
            Tuplet::resetNumberProperty(number);
            number->read(e);
            number->setVisible(t->visible());         //?? override saved property
            number->setTrack(t->track());
            // move property flags from _number back to tuplet
            for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN }) {
                t->setPropertyFlags(p, number->propertyFlags(p));
            }
        } else if (!EngravingItemRW::readProperties(t, e, ctx)) {
            e.unknown();
        }
    }

    t->setNumber(number);
    t->setBaseLen(baseLen);
    t->setRatio(ratio);

    Fraction f =  t->baseLen().fraction() * t->ratio().denominator();
    t->setTicks(f.reduced());
}

void TRead::read(Beam* b, XmlReader& e, ReadContext& ctx)
{
    if (b->score()->mscVersion() < 301) {
        b->setId(e.intAttribute("id"));
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "StemDirection") {
            PropertyRW::readProperty(b, e, ctx, Pid::STEM_DIRECTION);
        } else if (tag == "distribute") {
            e.skipCurrentElement(); // obsolete
        } else if (PropertyRW::readStyledProperty(b, tag, e, ctx)) {
        } else if (tag == "growLeft") {
            b->setGrowLeft(e.readDouble());
        } else if (tag == "growRight") {
            b->setGrowRight(e.readDouble());
        } else if (tag == "Fragment") {
            BeamFragment* f = new BeamFragment;
            int idx = (b->beamDirection() == DirectionV::AUTO || b->beamDirection() == DirectionV::DOWN) ? 0 : 1;
            b->setUserModified(true);
            double _spatium = b->spatium();
            while (e.readNextStartElement()) {
                const AsciiStringView tag1(e.name());
                if (tag1 == "y1") {
                    f->py1[idx] = e.readDouble() * _spatium;
                } else if (tag1 == "y2") {
                    f->py2[idx] = e.readDouble() * _spatium;
                } else {
                    e.unknown();
                }
            }
            b->addBeamFragment(f);
        } else if (tag == "l1" || tag == "l2") {      // ignore
            e.skipCurrentElement();
        } else if (tag == "subtype") {          // obsolete
            e.skipCurrentElement();
        } else if (tag == "y1" || tag == "y2") { // obsolete
            e.skipCurrentElement();
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Ambitus* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "head") {
            PropertyRW::readProperty(a, e, ctx, Pid::HEAD_GROUP);
        } else if (tag == "headType") {
            PropertyRW::readProperty(a, e, ctx, Pid::HEAD_TYPE);
        } else if (tag == "mirror") {
            PropertyRW::readProperty(a, e, ctx, Pid::MIRROR_HEAD);
        } else if (tag == "hasLine") {
            a->setHasLine(e.readInt());
        } else if (tag == "lineWidth") {
            PropertyRW::readProperty(a, e, ctx, Pid::LINE_WIDTH_SPATIUM);
        } else if (tag == "topPitch") {
            a->setTopPitch(e.readInt(), false);
        } else if (tag == "bottomPitch") {
            a->setBottomPitch(e.readInt(), false);
        } else if (tag == "topTpc") {
            a->setTopTpc(e.readInt(), false);
        } else if (tag == "bottomTpc") {
            a->setBottomTpc(e.readInt(), false);
        } else if (tag == "topAccidental") {
            while (e.readNextStartElement()) {
                if (e.name() == "Accidental") {
                    if (a->score()->mscVersion() < 301) {
                        compat::Read206::readAccidental206(a->topAccidental(), e);
                    } else {
                        TRead::read(a->topAccidental(), e, ctx);
                    }
                } else {
                    e.skipCurrentElement();
                }
            }
        } else if (tag == "bottomAccidental") {
            while (e.readNextStartElement()) {
                if (e.name() == "Accidental") {
                    if (a->score()->mscVersion() < 301) {
                        compat::Read206::readAccidental206(a->bottomAccidental(), e);
                    } else {
                        TRead::read(a->bottomAccidental(), e, ctx);
                    }
                } else {
                    e.skipCurrentElement();
                }
            }
        } else if (EngravingItemRW::readProperties(a, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Accidental* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "bracket") {
            int i = e.readInt();
            if (i == 0 || i == 1 || i == 2) {
                a->setBracket(AccidentalBracket(i));
            }
        } else if (tag == "subtype") {
            a->setSubtype(e.readAsciiText());
        } else if (tag == "role") {
            a->setRole(TConv::fromXml(e.readAsciiText(), AccidentalRole::AUTO));
        } else if (tag == "small") {
            a->setSmall(e.readInt());
        } else if (EngravingItemRW::readProperties(a, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Marker* m, XmlReader& e, ReadContext& ctx)
{
    MarkerType mt = MarkerType::SEGNO;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "label") {
            AsciiStringView s(e.readAsciiText());
            m->setLabel(String::fromAscii(s.ascii()));
            mt = TConv::fromXml(s, MarkerType::USER);
        } else if (!TextBaseRW::readProperties(m, e, ctx)) {
            e.unknown();
        }
    }
    m->setMarkerType(mt);
}

void TRead::read(Jump* j, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "jumpTo") {
            j->setJumpTo(e.readText());
        } else if (tag == "playUntil") {
            j->setPlayUntil(e.readText());
        } else if (tag == "continueAt") {
            j->setContinueAt(e.readText());
        } else if (tag == "playRepeats") {
            j->setPlayRepeats(e.readBool());
        } else if (!TextBaseRW::readProperties(j, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(MeasureNumber* n, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<MeasureNumberBase*>(n), xml, ctx);
}

void TRead::read(MeasureNumberBase* b, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<TextBase*>(b), xml, ctx);
}

void TRead::read(MMRestRange* r, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<MeasureNumberBase*>(r), xml, ctx);
}

void TRead::read(SystemDivider* d, XmlReader& e, ReadContext& ctx)
{
    if (e.attribute("type") == "left") {
        d->setDividerType(SystemDivider::Type::LEFT);

        SymId sym = SymNames::symIdByName(d->score()->styleSt(Sid::dividerLeftSym));
        d->setSym(sym, d->score()->engravingFont());
    } else {
        d->setDividerType(SystemDivider::Type::RIGHT);

        SymId sym = SymNames::symIdByName(d->score()->styleSt(Sid::dividerRightSym));
        d->setSym(sym, d->score()->engravingFont());
    }
    TRead::read(static_cast<Symbol*>(d), e, ctx);
}

void TRead::read(ActionIcon* i, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "action") {
            i->setAction(e.readText().toStdString(), 0);
        } else if (tag == "subtype") {
            i->setActionType(static_cast<ActionIconType>(e.readInt()));
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Arpeggio* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            a->setArpeggioType(TConv::fromXml(e.readAsciiText(), ArpeggioType::NORMAL));
        } else if (tag == "userLen1") {
            a->setUserLen1(e.readDouble() * a->spatium());
        } else if (tag == "userLen2") {
            a->setUserLen2(e.readDouble() * a->spatium());
        } else if (tag == "span") {
            a->setSpan(e.readInt());
        } else if (tag == "play") {
            a->setPlayArpeggio(e.readBool());
        } else if (tag == "timeStretch") {
            a->setStretch(e.readDouble());
        } else if (!EngravingItemRW::readProperties(a, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Articulation* a, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        if (!readProperties(a, xml, ctx)) {
            xml.unknown();
        }
    }
}

bool TRead::readProperties(Articulation* a, XmlReader& xml, ReadContext& ctx)
{
    const AsciiStringView tag(xml.name());

    if (tag == "subtype") {
        AsciiStringView s = xml.readAsciiText();
        ArticulationTextType textType = TConv::fromXml(s, ArticulationTextType::NO_TEXT);
        if (textType != ArticulationTextType::NO_TEXT) {
            a->setTextType(textType);
        } else {
            SymId id = SymNames::symIdByName(s);
            if (id == SymId::noSym) {
                id = compat::Read206::articulationNames2SymId206(s); // compatibility hack for "old" 3.0 scores
            }
            if (id == SymId::noSym || s == "ornamentMordentInverted") {   // SMuFL < 1.30
                id = SymId::ornamentMordent;
            }

            //! TODO Should be replaced with `ctx.mscoreVersion()`
            //! But at the moment, `ctx` is not set everywhere
            String programVersion = a->score()->mscoreVersion();
            if (!programVersion.isEmpty() && programVersion < u"3.6") {
                if (id == SymId::noSym || s == "ornamentMordent") {   // SMuFL < 1.30 and MuseScore < 3.6
                    id = SymId::ornamentShortTrill;
                }
            }
            a->setSymId(id);
        }
    } else if (tag == "channel") {
        a->setChannelName(xml.attribute("name"));
        xml.readNext();
    } else if (PropertyRW::readProperty(a, tag, xml, ctx, Pid::ARTICULATION_ANCHOR)) {
    } else if (tag == "direction") {
        PropertyRW::readProperty(a, xml, ctx, Pid::DIRECTION);
    } else if (tag == "ornamentStyle") {
        PropertyRW::readProperty(a, xml, ctx, Pid::ORNAMENT_STYLE);
    } else if (tag == "play") {
        a->setPlayArticulation(xml.readBool());
    } else if (tag == "offset") {
        if (a->score()->mscVersion() >= 400) {
            EngravingItemRW::readProperties(a, xml, ctx);
        } else {
            xml.skipCurrentElement();       // ignore manual layout in older scores
        }
    } else if (EngravingItemRW::readProperties(a, xml, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(Audio* a, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        if (e.name() == "path") {
            a->setPath(e.readText());
        } else {
            e.unknown();
        }
    }
}

void TRead::read(BagpipeEmbellishment* b, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            b->setEmbelType(TConv::fromXml(e.readAsciiText(), EmbellishmentType(0)));
        } else {
            e.unknown();
        }
    }
}

void TRead::read(BarLine* b, XmlReader& e, ReadContext& ctx)
{
    b->resetProperty(Pid::BARLINE_SPAN);
    b->resetProperty(Pid::BARLINE_SPAN_FROM);
    b->resetProperty(Pid::BARLINE_SPAN_TO);

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            b->setBarLineType(TConv::fromXml(e.readAsciiText(), BarLineType::NORMAL));
        } else if (tag == "span") {
            b->setSpanStaff(e.readBool());
        } else if (tag == "spanFromOffset") {
            b->setSpanFrom(e.readInt());
        } else if (tag == "spanToOffset") {
            b->setSpanTo(e.readInt());
        } else if (tag == "Articulation") {
            Articulation* a = Factory::createArticulation(b->score()->dummy()->chord());
            TRead::read(a, e, ctx);
            b->add(a);
        } else if (tag == "Symbol") {
            Symbol* s = Factory::createSymbol(b);
            s->setTrack(b->track());
            TRead::read(s, e, ctx);
            b->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = Factory::createImage(b);
                image->setTrack(b->track());
                TRead::read(image, e, ctx);
                b->add(image);
            }
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Bend* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (PropertyRW::readStyledProperty(b, tag, e, ctx)) {
        } else if (tag == "point") {
            PitchValue pv;
            pv.time    = e.intAttribute("time");
            pv.pitch   = e.intAttribute("pitch");
            pv.vibrato = e.intAttribute("vibrato");
            b->addPoint(pv);
            e.readNext();
        } else if (tag == "play") {
            b->setPlayBend(e.readBool());
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Box* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(b, e, ctx)) {
            e.unknown();
        }
    }

    if (b->score()->mscVersion() < 302) {
        b->setAutoSizeEnabled(false);    // disable auto-size for older scores by default.
    }
}

void TRead::read(HBox* b, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<Box*>(b), xml, ctx);
}

void TRead::read(VBox* b, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<Box*>(b), xml, ctx);
}

void TRead::read(FBox* b, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<Box*>(b), xml, ctx);
}

void TRead::read(TBox* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Text") {
            b->text()->read(e);
        } else if (TRead::readProperties(static_cast<Box*>(b), e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Box* b, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (tag == "height") {
        b->setBoxHeight(Spatium(e.readDouble()));
    } else if (tag == "width") {
        b->setBoxWidth(Spatium(e.readDouble()));
    } else if (tag == "topGap") {
        double gap = e.readDouble();
        b->setTopGap(Millimetre(gap));
        if (b->score()->mscVersion() >= 206) {
            b->setTopGap(Millimetre(gap * b->score()->spatium()));
        }
        b->setPropertyFlags(Pid::TOP_GAP, PropertyFlags::UNSTYLED);
    } else if (tag == "bottomGap") {
        double gap = e.readDouble();
        b->setBottomGap(Millimetre(gap));
        if (b->score()->mscVersion() >= 206) {
            b->setBottomGap(Millimetre(gap * b->score()->spatium()));
        }
        b->setPropertyFlags(Pid::BOTTOM_GAP, PropertyFlags::UNSTYLED);
    } else if (tag == "leftMargin") {
        b->setLeftMargin(e.readDouble());
    } else if (tag == "rightMargin") {
        b->setRightMargin(e.readDouble());
    } else if (tag == "topMargin") {
        b->setTopMargin(e.readDouble());
    } else if (tag == "bottomMargin") {
        b->setBottomMargin(e.readDouble());
    } else if (tag == "boxAutoSize") {
        b->setAutoSizeEnabled(e.readBool());
    } else if (tag == "Text") {
        Text* t;
        if (b->isTBox()) {
            t = toTBox(b)->text();
            t->read(e);
        } else {
            t = Factory::createText(b);
            t->read(e);
            if (t->empty()) {
                LOGD("read empty text");
            } else {
                b->add(t);
            }
        }
    } else if (tag == "Symbol") {
        Symbol* s = new Symbol(b);
        TRead::read(s, e, ctx);
        b->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(b);
            image->setTrack(ctx.track());
            TRead::read(image, e, ctx);
            b->add(image);
        }
    } else if (tag == "FretDiagram") {
        FretDiagram* f = Factory::createFretDiagram(b->score()->dummy()->segment());
        f->read(e);
        //! TODO Looks like a bug.
        //! The FretDiagram parent must be Segment
        //! there is a method: `Segment* segment() const { return toSegment(parent()); }`,
        //! but when we add it to Box, the parent will be rewritten.
        b->add(f);
    } else if (tag == "HBox") {
        // m_parent is used here (rather than system()) because explicit parent isn't set for this object
        // until it is fully read. m_parent is nonetheless valid and using it here matches MU3 behavior.
        // If we do not set the parent of this new box correctly, it will cause a crash on read()
        // because it needs access to the system it is being added to. (c.r. issue #14643)
        if (b->parent() && b->parent()->isSystem()) {
            HBox* hb = new HBox(toSystem(b->parent()));
            hb->read(e);
            //! TODO Looks like a bug.
            //! The HBox parent must be System
            //! there is a method: `System* system() const { return (System*)parent(); }`,
            //! but when we add it to Box, the parent will be rewritten.
            b->add(hb);
        }
    } else if (tag == "VBox") {
        if (b->parent() && b->parent()->isSystem()) {
            VBox* vb = new VBox(toSystem(b->parent()));
            vb->read(e);
            //! TODO Looks like a bug.
            //! The VBox parent must be System
            //! there is a method: `System* system() const { return (System*)parent(); }`,
            //! but when we add it to Box, the parent will be rewritten.
            b->add(vb);
        }
    } else if (TRead::readProperties(static_cast<MeasureBase*>(b), e, ctx)) {
    } else {
        return false;
    }
    return true;
}

bool TRead::readProperties(MeasureBase* b, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (tag == "LayoutBreak") {
        LayoutBreak* lb = Factory::createLayoutBreak(b);
        lb->read(e);
        bool doAdd = true;
        switch (lb->layoutBreakType()) {
        case LayoutBreakType::LINE:
            if (b->lineBreak()) {
                doAdd = false;
            }
            break;
        case LayoutBreakType::PAGE:
            if (b->pageBreak()) {
                doAdd = false;
            }
            break;
        case LayoutBreakType::SECTION:
            if (b->sectionBreak()) {
                doAdd = false;
            }
            break;
        case LayoutBreakType::NOBREAK:
            if (b->noBreak()) {
                doAdd = false;
            }
            break;
        }
        if (doAdd) {
            b->add(lb);
            b->cleanupLayoutBreaks(false);
        } else {
            delete lb;
        }
    } else if (tag == "StaffTypeChange") {
        StaffTypeChange* stc = Factory::createStaffTypeChange(b);
        stc->setTrack(e.context()->track());
        stc->setParent(b);
        stc->read(e);
        b->add(stc);
    } else if (EngravingItemRW::readProperties(b, e, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(Bracket* b, XmlReader& e, ReadContext& ctx)
{
    BracketItem* bi = Factory::createBracketItem(b->score()->dummy());
    bi->setBracketType(TConv::fromXml(e.asciiAttribute("type"), BracketType::NORMAL));

    while (e.readNextStartElement()) {
        if (e.name() == "level") {
            bi->setColumn(e.readInt());
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }

    b->setBracketItem(bi);
}

void TRead::read(Breath* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {                 // obsolete
            SymId symId = SymId::noSym;
            switch (e.readInt()) {
            case 0:
            case 1:
                symId = SymId::breathMarkComma;
                break;
            case 2:
                symId = SymId::caesuraCurved;
                break;
            case 3:
                symId = SymId::caesura;
                break;
            }
            b->setSymId(symId);
        } else if (tag == "symbol") {
            b->setSymId(SymNames::symIdByName(e.readAsciiText()));
        } else if (tag == "pause") {
            b->setPause(e.readDouble());
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(BSymbol* sym, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag = e.name();

    if (EngravingItemRW::readProperties(sym, e, ctx)) {
        return true;
    } else if (tag == "systemFlag") {
        sym->setSystemFlag(e.readInt());
    } else if (tag == "Symbol") {
        Symbol* s = Factory::createSymbol(sym);
        TRead::read(s, e, ctx);
        sym->add(s);
    } else if (tag == "FSymbol") {
        FSymbol* s = Factory::createFSymbol(sym);
        TRead::read(s, e, ctx);
        sym->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(sym);
            TRead::read(image, e, ctx);
            sym->add(image);
        }
    } else {
        return false;
    }
    return true;
}

void TRead::read(Symbol* sym, XmlReader& e, ReadContext& ctx)
{
    SymId symId = SymId::noSym;
    String fontName;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "name") {
            String val(e.readText());
            symId = SymNames::symIdByName(val);
            if (val != "noSym" && symId == SymId::noSym) {
                // if symbol name not found, fall back to user names
                // TODO: does it make sense? user names are probably localized
                symId = SymNames::symIdByUserName(val);
                if (symId == SymId::noSym) {
                    LOGD("unknown symbol <%s>, falling back to no symbol", muPrintable(val));
                    // set a default symbol, or layout() will crash
                    symId = SymId::noSym;
                }
            }
            sym->setSym(symId);
        } else if (tag == "font") {
            fontName = e.readText();
        } else if (tag == "Symbol") {
            Symbol* s = new Symbol(sym);
            TRead::read(s, e, ctx);
            sym->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = new Image(sym);
                TRead::read(image, e, ctx);
                sym->add(image);
            }
        } else if (tag == "small" || tag == "subtype") {    // obsolete
            e.skipCurrentElement();
        } else if (!TRead::readProperties(static_cast<BSymbol*>(sym), e, ctx)) {
            e.unknown();
        }
    }

    std::shared_ptr<IEngravingFont> scoreFont = nullptr;
    if (!fontName.empty()) {
        scoreFont = ctx.engravingFonts()->fontByName(fontName.toStdString());
    }

    sym->setPos(PointF());
    sym->setSym(symId, scoreFont);
}

void TRead::read(FSymbol* sym, XmlReader& e, ReadContext& ctx)
{
    mu::draw::Font font = sym->font();
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "font") {
            font.setFamily(e.readText(), draw::Font::Type::Unknown);
        } else if (tag == "fontsize") {
            font.setPointSizeF(e.readDouble());
        } else if (tag == "code") {
            sym->setCode(e.readInt());
        } else if (!TRead::readProperties(static_cast<BSymbol*>(sym), e, ctx)) {
            e.unknown();
        }
    }

    sym->setPos(PointF());
    sym->setFont(font);
}

void TRead::read(Chord* ch, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (TRead::readProperties(ch, e, ctx)) {
        } else {
            e.unknown();
        }
    }

    //! TODO Should be replaced with `ctx.mscVersion()`
    //! But at the moment, `ctx` is not set everywhere
    int mscVersion = ch->score()->mscVersion();

    // Reset horizontal offset of grace notes when migrating from before 4.0
    if (ch->isGrace() && mscVersion < 400) {
        ch->rxoffset() = 0;
    }
}

bool TRead::readProperties(Chord* ch, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "Note") {
        Note* note = Factory::createNote(ch);
        // the note needs to know the properties of the track it belongs to
        note->setTrack(ch->track());
        note->setParent(ch);
        TRead::read(note, e, ctx);
        ch->add(note);
    } else if (TRead::readProperties(static_cast<ChordRest*>(ch), e, ctx)) {
    } else if (tag == "Stem") {
        Stem* s = Factory::createStem(ch);
        TRead::read(s, e, ctx);
        ch->add(s);
    } else if (tag == "Hook") {
        Hook* hook = new Hook(ch);
        TRead::read(hook, e, ctx);
        ch->setHook(hook);
        ch->add(hook);
    } else if (tag == "appoggiatura") {
        ch->setNoteType(NoteType::APPOGGIATURA);
        e.readNext();
    } else if (tag == "acciaccatura") {
        ch->setNoteType(NoteType::ACCIACCATURA);
        e.readNext();
    } else if (tag == "grace4") {
        ch->setNoteType(NoteType::GRACE4);
        e.readNext();
    } else if (tag == "grace16") {
        ch->setNoteType(NoteType::GRACE16);
        e.readNext();
    } else if (tag == "grace32") {
        ch->setNoteType(NoteType::GRACE32);
        e.readNext();
    } else if (tag == "grace8after") {
        ch->setNoteType(NoteType::GRACE8_AFTER);
        e.readNext();
    } else if (tag == "grace16after") {
        ch->setNoteType(NoteType::GRACE16_AFTER);
        e.readNext();
    } else if (tag == "grace32after") {
        ch->setNoteType(NoteType::GRACE32_AFTER);
        e.readNext();
    } else if (tag == "StemSlash") {
        StemSlash* ss = Factory::createStemSlash(ch);
        TRead::read(ss, e, ctx);
        ch->add(ss);
    } else if (PropertyRW::readProperty(ch, tag, e, ctx, Pid::STEM_DIRECTION)) {
    } else if (tag == "noStem") {
        ch->setNoStem(e.readInt());
    } else if (tag == "Arpeggio") {
        Arpeggio* arpeggio = Factory::createArpeggio(ch);
        arpeggio->setTrack(ch->track());
        TRead::read(arpeggio, e, ctx);
        arpeggio->setParent(ch);
        ch->setArpeggio(arpeggio);
    } else if (tag == "Tremolo") {
        Tremolo* tremolo = Factory::createTremolo(ch);
        tremolo->setTrack(ch->track());
        TRead::read(tremolo, e, ctx);
        tremolo->setParent(ch);
        tremolo->setDurationType(ch->durationType());
        ch->setTremolo(tremolo, false);
    } else if (tag == "tickOffset") {      // obsolete
    } else if (tag == "ChordLine") {
        ChordLine* cl = Factory::createChordLine(ch);
        TRead::read(cl, e, ctx);
        ch->add(cl);
    } else {
        return false;
    }
    return true;
}

bool TRead::readProperties(ChordRest* ch, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    //! TODO Should be replaced with `ctx.mscVersion()`
    //! But at the moment, `ctx` is not set everywhere
    int mscVersion = ch->score()->mscVersion();

    if (tag == "durationType") {
        ch->setDurationType(TConv::fromXml(e.readAsciiText(), DurationType::V_QUARTER));
        if (ch->actualDurationType().type() != DurationType::V_MEASURE) {
            if (mscVersion < 112 && (ch->type() == ElementType::REST)
                &&            // for backward compatibility, convert V_WHOLE rests to V_MEASURE
                              // if long enough to fill a measure.
                              // OTOH, freshly created (un-initialized) rests have numerator == 0 (< 4/4)
                              // (see Fraction() constructor in fraction.h; this happens for instance
                              // when pasting selection from clipboard): they should not be converted
                ch->ticks().numerator() != 0
                &&            // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                (ch->actualDurationType() == DurationType::V_WHOLE && ch->ticks() <= Fraction(4, 4))) {
                // old pre 2.0 scores: convert
                ch->setDurationType(DurationType::V_MEASURE);
            } else {    // not from old score: set duration fraction from duration type
                ch->setTicks(ch->actualDurationType().fraction());
            }
        } else {
            if (mscVersion <= 114) {
                SigEvent event = ch->score()->sigmap()->timesig(e.context()->tick());
                ch->setTicks(event.timesig());
            }
        }
    } else if (tag == "BeamMode") {
        ch->setBeamMode(TConv::fromXml(e.readAsciiText(), BeamMode::AUTO));
    } else if (tag == "Articulation") {
        Articulation* atr = Factory::createArticulation(ch);
        atr->setTrack(ch->track());
        TRead::read(atr, e, ctx);
        ch->add(atr);
    } else if (tag == "leadingSpace" || tag == "trailingSpace") {
        LOGD("ChordRest: %s obsolete", tag.ascii());
        e.skipCurrentElement();
    } else if (tag == "small") {
        ch->setSmall(e.readInt());
    } else if (tag == "duration") {
        ch->setTicks(e.readFraction());
    } else if (tag == "ticklen") {      // obsolete (version < 1.12)
        int mticks = ch->score()->sigmap()->timesig(e.context()->tick()).timesig().ticks();
        int i = e.readInt();
        if (i == 0) {
            i = mticks;
        }
        if ((ch->type() == ElementType::REST) && (mticks == i)) {
            ch->setDurationType(DurationType::V_MEASURE);
            ch->setTicks(Fraction::fromTicks(i));
        } else {
            Fraction f = Fraction::fromTicks(i);
            ch->setTicks(f);
            ch->setDurationType(TDuration(f));
        }
    } else if (tag == "dots") {
        ch->setDots(e.readInt());
    } else if (tag == "staffMove") {
        ch->setStaffMove(e.readInt());
        if (ch->vStaffIdx() < ch->part()->staves().front()->idx() || ch->vStaffIdx() > ch->part()->staves().back()->idx()) {
            ch->setStaffMove(0);
        }
    } else if (tag == "Spanner") {
        Spanner::readSpanner(e, ch, ch->track());
    } else if (tag == "Lyrics") {
        Lyrics* lyr = Factory::createLyrics(ch);
        lyr->setTrack(e.context()->track());
        TRead::read(lyr, e, ctx);
        ch->add(lyr);
    } else if (tag == "pos") {
        PointF pt = e.readPoint();
        ch->setOffset(pt * ch->spatium());
    }
//      else if (tag == "offset")
//            DurationElement::readProperties(e);
    else if (!EngravingItemRW::readProperties(ch, e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(ChordLine* l, XmlReader& e, ReadContext& ctx)
{
    l->setPath(PainterPath());
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Path") {
            PainterPath path;
            PointF curveTo;
            PointF p1;
            int state = 0;
            while (e.readNextStartElement()) {
                const AsciiStringView nextTag(e.name());
                if (nextTag == "Element") {
                    int type = e.intAttribute("type");
                    double x  = e.doubleAttribute("x");
                    double y  = e.doubleAttribute("y");
                    switch (PainterPath::ElementType(type)) {
                    case PainterPath::ElementType::MoveToElement:
                        path.moveTo(x, y);
                        break;
                    case PainterPath::ElementType::LineToElement:
                        path.lineTo(x, y);
                        break;
                    case PainterPath::ElementType::CurveToElement:
                        curveTo.rx() = x;
                        curveTo.ry() = y;
                        state = 1;
                        break;
                    case PainterPath::ElementType::CurveToDataElement:
                        if (state == 1) {
                            p1.rx() = x;
                            p1.ry() = y;
                            state = 2;
                        } else if (state == 2) {
                            path.cubicTo(curveTo, p1, PointF(x, y));
                            state = 0;
                        }
                        break;
                    }
                    e.skipCurrentElement();           //needed to go to next EngravingItem in Path
                } else {
                    e.unknown();
                }
            }
            l->setPath(path);
            l->setModified(true);
        } else if (tag == "subtype") {
            l->setChordLineType(TConv::fromXml(e.readAsciiText(), ChordLineType::NOTYPE));
        } else if (tag == "straight") {
            l->setStraight(e.readInt());
        } else if (tag == "wavy") {
            l->setWavy(e.readInt());
        } else if (tag == "lengthX") {
            l->setLengthX(e.readInt());
        } else if (tag == "lengthY") {
            l->setLengthY(e.readInt());
        } else if (tag == "offset" && l->score()->mscVersion() < 400) { // default positions has changed in 4.0 so ignore previous offset
            e.skipCurrentElement();
        } else if (!EngravingItemRW::readProperties(l, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Clef* c, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "concertClefType") {
            c->setConcertClef(TConv::fromXml(e.readAsciiText(), ClefType::G));
        } else if (tag == "transposingClefType") {
            c->setTransposingClef(TConv::fromXml(e.readAsciiText(), ClefType::G));
        } else if (tag == "showCourtesyClef") {
            c->setShowCourtesy(e.readInt());
        } else if (tag == "forInstrumentChange") {
            c->setForInstrumentChange(e.readBool());
        } else if (!EngravingItemRW::readProperties(c, e, ctx)) {
            e.unknown();
        }
    }

    if (c->clefType() == ClefType::INVALID) {
        c->setClefType(ClefType::G);
    }
}

void TRead::read(Fingering* f, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!TextBaseRW::readProperties(f, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Glissando* g, XmlReader& e, ReadContext& ctx)
{
    g->eraseSpannerSegments();

    if (g->score()->mscVersion() < 301) {
        ctx.addSpanner(e.intAttribute("id", -1), g);
    }

    g->setShowText(false);
    while (e.readNextStartElement()) {
        const AsciiStringView tag = e.name();
        if (tag == "text") {
            g->setShowText(true);
            PropertyRW::readProperty(g, e, ctx, Pid::GLISS_TEXT);
        } else if (tag == "subtype") {
            g->setGlissandoType(TConv::fromXml(e.readAsciiText(), GlissandoType::STRAIGHT));
        } else if (tag == "glissandoStyle") {
            PropertyRW::readProperty(g, e, ctx, Pid::GLISS_STYLE);
        } else if (tag == "easeInSpin") {
            g->setEaseIn(e.readInt());
        } else if (tag == "easeOutSpin") {
            g->setEaseOut(e.readInt());
        } else if (tag == "play") {
            g->setPlayGlissando(e.readBool());
        } else if (PropertyRW::readStyledProperty(g, tag, e, ctx)) {
        } else if (!TRead::readProperties(static_cast<SLine*>(g), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(GradualTempoChange* c, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        const AsciiStringView tag(xml.name());

        if (PropertyRW::readProperty(c, tag, xml, ctx, Pid::LINE_WIDTH)) {
            c->setPropertyFlags(Pid::LINE_WIDTH, PropertyFlags::UNSTYLED);
            continue;
        }

        if (PropertyRW::readProperty(c, tag, xml, ctx, Pid::TEMPO_CHANGE_TYPE)) {
            continue;
        }

        if (PropertyRW::readProperty(c, tag, xml, ctx, Pid::TEMPO_EASING_METHOD)) {
            continue;
        }

        if (PropertyRW::readProperty(c, tag, xml, ctx, Pid::TEMPO_CHANGE_FACTOR)) {
            continue;
        }

        if (!readProperties(static_cast<TextLineBase*>(c), xml, ctx)) {
            xml.unknown();
        }
    }
}

void TRead::read(Groups* g, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Node") {
            GroupNode n;
            n.pos    = e.intAttribute("pos");
            n.action = e.intAttribute("action");
            g->addNode(n);
            e.skipCurrentElement();
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Hairpin* h, XmlReader& e, ReadContext& ctx)
{
    h->eraseSpannerSegments();

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            h->setHairpinType(HairpinType(e.readInt()));
        } else if (PropertyRW::readStyledProperty(h, tag, e, ctx)) {
        } else if (tag == "hairpinCircledTip") {
            h->setHairpinCircledTip(e.readInt());
        } else if (tag == "veloChange") {
            h->setVeloChange(e.readInt());
        } else if (tag == "dynType") {
            h->setDynRange(TConv::fromXml(e.readAsciiText(), DynamicRange::STAFF));
        } else if (tag == "useTextLine") {        // obsolete
            e.readInt();
            if (h->hairpinType() == HairpinType::CRESC_HAIRPIN) {
                h->setHairpinType(HairpinType::CRESC_LINE);
            } else if (h->hairpinType() == HairpinType::DECRESC_HAIRPIN) {
                h->setHairpinType(HairpinType::DECRESC_LINE);
            }
        } else if (tag == "singleNoteDynamics") {
            h->setSingleNoteDynamics(e.readBool());
        } else if (tag == "veloChangeMethod") {
            h->setVeloChangeMethod(TConv::fromXml(e.readAsciiText(), ChangeMethod::NORMAL));
        } else if (!readProperties(static_cast<TextLineBase*>(h), e, ctx)) {
            e.unknown();
        }
    }

    h->styleChanged();
}

void TRead::read(Hook* h, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<Symbol*>(h), xml, ctx);
}

void TRead::read(LayoutBreak* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            PropertyRW::readProperty(b, e, ctx, Pid::LAYOUT_BREAK);
        } else if (tag == "pause") {
            PropertyRW::readProperty(b, e, ctx, Pid::PAUSE);
        } else if (tag == "startWithLongNames") {
            PropertyRW::readProperty(b, e, ctx, Pid::START_WITH_LONG_NAMES);
        } else if (tag == "startWithMeasureOne") {
            PropertyRW::readProperty(b, e, ctx, Pid::START_WITH_MEASURE_ONE);
        } else if (tag == "firstSystemIndentation"
                   || tag == "firstSystemIdentation" /* pre-4.0 typo */) {
            PropertyRW::readProperty(b, e, ctx, Pid::FIRST_SYSTEM_INDENTATION);
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
    b->layout0();
}

void TRead::read(Lyrics* l, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!TRead::readProperties(l, e, ctx)) {
            e.unknown();
        }
    }
    if (!l->isStyled(Pid::OFFSET) && !ctx.pasteMode()) {
        // fix offset for pre-3.1 scores
        // 3.0: y offset was meaningless if autoplace is set
        String version = ctx.mscoreVersion();
        if (l->autoplace() && !version.isEmpty() && version < u"3.1") {
            PointF off = l->propertyDefault(Pid::OFFSET).value<PointF>();
            l->ryoffset() = off.y();
        }
    }
}

bool TRead::readProperties(Lyrics* l, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "no") {
        l->setNo(e.readInt());
    } else if (tag == "syllabic") {
        l->setSyllabic(TConv::fromXml(e.readAsciiText(), LyricsSyllabic::SINGLE));
    } else if (tag == "ticks") {          // obsolete
        l->setTicks(e.readFraction());     // will fall back to reading integer ticks on older scores
    } else if (tag == "ticks_f") {
        l->setTicks(e.readFraction());
    } else if (PropertyRW::readProperty(l, tag, e, ctx, Pid::PLACEMENT)) {
    } else if (!TextBaseRW::readProperties(l, e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(Note* n, XmlReader& e, ReadContext& ctx)
{
    n->setTpc1(Tpc::TPC_INVALID);
    n->setTpc2(Tpc::TPC_INVALID);

    while (e.readNextStartElement()) {
        if (TRead::readProperties(n, e, ctx)) {
        } else {
            e.unknown();
        }
    }

    n->setupAfterRead(ctx.tick(), ctx.pasteMode());
}

bool TRead::readProperties(Note* n, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "pitch") {
        n->setPitch(std::clamp(e.readInt(), 0, 127), false);
    } else if (tag == "tpc") {
        int tcp = e.readInt();
        n->setTpc1(tcp);
        n->setTpc2(tcp);
    } else if (tag == "track") {          // for performance
        n->setTrack(e.readInt());
    } else if (tag == "Accidental") {
        Accidental* a = Factory::createAccidental(n);
        a->setTrack(n->track());
        TRead::read(a, e, ctx);
        n->add(a);
    } else if (tag == "Spanner") {
        Spanner::readSpanner(e, n, n->track());
    } else if (tag == "tpc2") {
        n->setTpc2(e.readInt());
    } else if (tag == "small") {
        n->setSmall(e.readInt());
    } else if (tag == "mirror") {
        PropertyRW::readProperty(n, e, ctx, Pid::MIRROR_HEAD);
    } else if (tag == "dotPosition") {
        PropertyRW::readProperty(n, e, ctx, Pid::DOT_POSITION);
    } else if (tag == "fixed") {
        n->setFixed(e.readBool());
    } else if (tag == "fixedLine") {
        n->setFixedLine(e.readInt());
    } else if (tag == "headScheme") {
        PropertyRW::readProperty(n, e, ctx, Pid::HEAD_SCHEME);
    } else if (tag == "head") {
        PropertyRW::readProperty(n, e, ctx, Pid::HEAD_GROUP);
    } else if (tag == "velocity") {
        n->setUserVelocity(e.readInt());
    } else if (tag == "play") {
        n->setPlay(e.readInt());
    } else if (tag == "tuning") {
        n->setTuning(e.readDouble());
    } else if (tag == "fret") {
        n->setFret(e.readInt());
    } else if (tag == "string") {
        n->setString(e.readInt());
    } else if (tag == "ghost") {
        n->setGhost(e.readInt());
    } else if (tag == "dead") {
        n->setDeadNote(e.readInt());
    } else if (tag == "headType") {
        PropertyRW::readProperty(n, e, ctx, Pid::HEAD_TYPE);
    } else if (tag == "veloType") {
        PropertyRW::readProperty(n, e, ctx, Pid::VELO_TYPE);
    } else if (tag == "line") {
        n->setLine(e.readInt());
    } else if (tag == "Fingering") {
        Fingering* f = Factory::createFingering(n);
        f->setTrack(n->track());
        TRead::read(f, e, ctx);
        n->add(f);
    } else if (tag == "Symbol") {
        Symbol* s = new Symbol(n);
        s->setTrack(n->track());
        TRead::read(s, e, ctx);
        n->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(n);
            image->setTrack(n->track());
            TRead::read(image, e, ctx);
            n->add(image);
        }
    } else if (tag == "Bend") {
        Bend* b = Factory::createBend(n);
        b->setTrack(n->track());
        TRead::read(b, e, ctx);
        n->add(b);
    } else if (tag == "NoteDot") {
        NoteDot* dot = Factory::createNoteDot(n);
        TRead::read(dot, e, ctx);
        n->add(dot);
    } else if (tag == "Events") {
        NoteEventList playEvents;
        while (e.readNextStartElement()) {
            const AsciiStringView t(e.name());
            if (t == "Event") {
                NoteEvent ne;
                ne.read(e);
                playEvents.push_back(ne);
            } else {
                e.unknown();
            }
        }
        n->setPlayEvents(playEvents);
        if (n->chord()) {
            n->chord()->setPlayEventType(PlayEventType::User);
        }
    } else if (tag == "offset") {
        EngravingItemRW::readProperties(n, e, ctx);
    } else if (tag == "ChordLine" && n->chord()) {
        ChordLine* cl = Factory::createChordLine(n->chord());
        cl->setNote(n);
        TRead::read(cl, e, ctx);
        n->chord()->add(cl);
    } else if (EngravingItemRW::readProperties(n, e, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(NoteDot* d, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (e.name() == "name") {      // obsolete
            e.readText();
        } else if (e.name() == "subtype") {     // obsolete
            e.readText();
        } else if (!EngravingItemRW::readProperties(d, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(SLine* l, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "tick2") {                  // obsolete
        if (l->tick() == Fraction(-1, 1)) {   // not necessarily set (for first note of score?) #30151
            l->setTick(ctx.tick());
        }
        l->setTick2(Fraction::fromTicks(e.readInt()));
    } else if (tag == "tick") {           // obsolete
        l->setTick(Fraction::fromTicks(e.readInt()));
    } else if (tag == "ticks") {
        l->setTicks(Fraction::fromTicks(e.readInt()));
    } else if (tag == "Segment") {
        LineSegment* ls = l->createLineSegment(l->score()->dummy()->system());
        ls->setTrack(l->track());     // needed in read to get the right staff mag
        ls->read(e);
        l->add(ls);
        ls->setVisible(l->visible());
    } else if (tag == "length") {
        l->setLen(e.readDouble());
    } else if (tag == "diagonal") {
        l->setDiagonal(e.readInt());
    } else if (tag == "anchor") {
        l->setAnchor(SLine::Anchor(e.readInt()));
    } else if (tag == "lineWidth") {
        l->setLineWidth(Millimetre(e.readDouble() * l->spatium()));
    } else if (PropertyRW::readProperty(l, tag, e, ctx, Pid::LINE_STYLE)) {
    } else if (tag == "dashLineLength") {
        l->setDashLineLen(e.readDouble());
    } else if (tag == "dashGapLength") {
        l->setDashGapLen(e.readDouble());
    } else if (tag == "lineColor") {
        l->setLineColor(e.readColor());
    } else if (tag == "color") {
        l->setLineColor(e.readColor());
    } else if (!readProperties(static_cast<Spanner*>(l), e, ctx)) {
        return false;
    }
    return true;
}

bool TRead::readProperties(Spanner* s, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (ctx.pasteMode()) {
        if (tag == "ticks_f") {
            s->setTicks(e.readFraction());
            return true;
        }
    }
    return EngravingItemRW::readProperties(s, e, ctx);
}

void TRead::read(Stem* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!TRead::readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Stem* s, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (PropertyRW::readProperty(s, tag, e, ctx, Pid::USER_LEN)) {
    } else if (PropertyRW::readStyledProperty(s, tag, e, ctx)) {
    } else if (EngravingItemRW::readProperties(s, e, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(StemSlash* s, XmlReader& xml, ReadContext& ctx)
{
    EngravingItemRW::read(s, xml, ctx);
}

void TRead::read(Tremolo* t, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            t->setTremoloType(TConv::fromXml(e.readAsciiText(), TremoloType::INVALID_TREMOLO));
        }
        // Style needs special handling other than readStyledProperty()
        // to avoid calling customStyleApplicable() in setProperty(),
        // which cannot be called now because durationType() isn't defined yet.
        else if (tag == "strokeStyle") {
            t->setStyle(TremoloStyle(e.readInt()));
            t->setPropertyFlags(Pid::TREMOLO_STYLE, PropertyFlags::UNSTYLED);
        } else if (tag == "Fragment") {
            BeamFragment f = BeamFragment();
            int idx = (t->direction() == DirectionV::AUTO || t->direction() == DirectionV::DOWN) ? 0 : 1;
            t->setUserModified(t->direction(), true);
            double _spatium = t->spatium();
            while (e.readNextStartElement()) {
                const AsciiStringView tag1(e.name());
                if (tag1 == "y1") {
                    f.py1[idx] = e.readDouble() * _spatium;
                } else if (tag1 == "y2") {
                    f.py2[idx] = e.readDouble() * _spatium;
                } else {
                    e.unknown();
                }
            }
            t->setBeamFragment(f);
        } else if (PropertyRW::readStyledProperty(t, tag, e, ctx)) {
        } else if (!EngravingItemRW::readProperties(t, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(TextLineBase* b, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    for (Pid i : TextLineBase::textLineBasePropertyIds()) {
        if (PropertyRW::readProperty(b, tag, e, ctx, i)) {
            b->setPropertyFlags(i, PropertyFlags::UNSTYLED);
            return true;
        }
    }
    return readProperties(static_cast<SLine*>(b), e, ctx);
}
