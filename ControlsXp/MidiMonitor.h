#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <list>
#include "../se_sdk3/mp_sdk_audio.h"
#include "../se_sdk3/mp_midi.h"
#include "../se_sdk3/it_enum_list.h"

using namespace GmpiMidi;

class MonitorBase : public MpBase2
{
protected:
	MidiInPin pinMIDIIn;
	StringOutPin pinDispOut;

	int64_t samplesPassed;
	float secondsPassed;

	const int lineLength = 40;
	std::list<std::wstring> lines;
	bool noteStatus[GmpiMidi::MidiChannels::MIDI_ChannelCount][GmpiMidi::MidiLimits::MIDI_KeyCount];
	int64_t noteStatusTimestamps[GmpiMidi::MidiChannels::MIDI_ChannelCount][GmpiMidi::MidiLimits::MIDI_KeyCount];
	std::vector<std::wstring> CONTROLLER_DESCRIPTION;
	static const wchar_t* CONTROLLER_ENUM_LIST;

public:
	MonitorBase()
	{
		memset(noteStatus, sizeof(noteStatus), 0);
		memset(noteStatusTimestamps, sizeof(noteStatusTimestamps), 0);
		samplesPassed = 0;
		lines.push_back(L"Sound ON");

		{
			CONTROLLER_DESCRIPTION.assign(128, L"");

			std::wstring enumList(CONTROLLER_ENUM_LIST);
			it_enum_list itr(enumList);

			for (itr.First(); !itr.IsDone(); itr.Next())
			{
				enum_entry* e = itr.CurrentItem();
				if (e->value >= 0) // ignore "none"
				{
					CONTROLLER_DESCRIPTION[e->value] = e->text;
				}
			}

			// provide default descriptions
			for (int i = 0; i < 128; i++)
			{
				if (CONTROLLER_DESCRIPTION[i].empty())
				{
					std::wostringstream oss;
					oss << L"CC" << std::setw(3) << i;
					CONTROLLER_DESCRIPTION[i] = oss.str();
				}
			}
		}
	}

	void subProcess(int sampleFrames)
	{
		samplesPassed += sampleFrames;
	}

	void onGraphStart() override
	{
		MpBase2::onGraphStart();

		setSubProcess(&MonitorBase::subProcess);

		setSleep(false);

		pinDispOut = lines.back();
	}
	/* never called, no non-midi pins
	virtual void onSetPins(void) override
	{
	// Set processing method.
	setSubProcess(&MidiMonitor::subProcess);

	setSleep(false);

	pinDispOut = lines.back();
	}
	*/

/*
	std::wstring generateTimeString()
	{
		int rate = (int)getSampleRate();

		int64_t currentSample = samplesPassed + getBlockPosition();
		int64_t seconds = (int64_t)currentSample / rate;
		int64_t samples = currentSample - seconds * rate;
		int64_t minutes = seconds / 60;

		std::wstring samplesText = to_wstring(samples) + L"s";
		std::wstring secondsText = to_wstring(seconds) + L"sec";
		std::wstring minutesText = to_wstring(minutes) + L"min";

		return L" " + minutesText + secondsText + samplesText;
	}
*/
	void onMidiMessage(int pin, const unsigned char* midiMessage, int size) override
	{
		//		wstring newMessage = midiToString.convert(size, midiMessage);
		//		wstring timeText = generateTimeString();

		//		msg = newMessage + timeText + L"\n" + msg + L"\n";

		std::wstring newMessage;
		{
			int stat, byte1, byte2, chan; // 3 bytes of MIDI message
			chan = midiMessage[0] & 0x0f;
			stat = midiMessage[0] & 0xf0;
			bool is_system_msg = (stat & MIDI_SystemMessage) == MIDI_SystemMessage;
			byte1 = midiMessage[1] & 0x7F;
			byte2 = midiMessage[2] & 0x7F;
			//	_RPT3(_CRT_WARN,"ug_midi_monitor::OnMidiData(%x,%x,%x)\n", (int)stat,(int)byte1,(int)byte2);
			//	if( chan == channel || channel == -1 || is_system_msg ) //-1 = All channels
			{
				//			newMessage.Format("MIDI MONITOR :%2d :", chan );
				// Note offs can be note_on vel=0
				if (byte2 == 0 && stat == MIDI_NoteOn)
					stat = MIDI_NoteOff;

				std::wostringstream oss;

				switch (stat)
				{
				case MIDI_NoteOn:
					// Note On and Note-Off at same time?
					if (!noteStatus[chan][byte1] && noteStatusTimestamps[chan][byte1] == samplesPassed)
					{
						oss << L"!!AMBIGUOUS!! ";
					}
					//			newMessage.Format((L"Note On  (%3d,%3d)"), byte1, byte2 );
					oss << L"Note On  (" << byte1 << "," << byte2 << ")";
					newMessage = oss.str();
					noteStatus[chan][byte1] = true;
					noteStatusTimestamps[chan][byte1] = samplesPassed;
					break;

				case MIDI_NoteOff:
					// Note On and Note-Off at same time?
					if (noteStatus[chan][byte1] && noteStatusTimestamps[chan][byte1] == samplesPassed)
					{
						oss << L"!!AMBIGUOUS!! ";
					}

					// newMessage.Format((L"Note Off (%3d,%3d)"), byte1, byte2 );
					oss << L"Note Off  (" << byte1 << "," << byte2 << ")";
					newMessage = oss.str();
					noteStatus[chan][byte1] = false;
					noteStatusTimestamps[chan][byte1] = samplesPassed;
					break;

				case MIDI_PolyAfterTouch:
					oss << L"Aftertouch  (" << byte1 << "," << byte2 << ")";
					newMessage = oss.str();
					break;

				case MIDI_ControlChange:
				{
					if (byte1 >= 0 && byte1 < 128)
					{
						std::wstring desc = CONTROLLER_DESCRIPTION[byte1];
																		//newMessage.Format((L"CC%s (%d)"), desc, byte2 );
																		// remove CC number from description. TRim to 20 char.
						size_t p = desc.find(L'-');

						if (p != std::string::npos)
						{
							//							desc = Left(Right(desc, desc.size() - p), 20);
							desc = desc.substr(p, (std::min)(desc.size() - p, (size_t)20));
						}

						//newMessage.Format((L"CC%d %d %s"), byte1, byte2, desc );
						oss << L"CC" << byte1 << ":" << std::setw(3) << std::right << byte2 << " " << desc;
						newMessage = oss.str();
					}
					else
					{
						newMessage = (L"Cntlr: ILEGAL CNTRLR");
					}
				}
				break;

				case MIDI_ProgramChange:
					//newMessage.Format((L"Prg change (%3d)"), byte1 + 1 );
					oss << L"Prg change  (" << (byte1 + 1) << ")";
					newMessage = oss.str();
					break;

				case MIDI_ChannelPressue:
					//newMessage.Format((L"Channel Pressure (%3d)"), byte1);
					oss << L"Channel Pressure  (" << byte1 << ")";
					newMessage = oss.str();
					break;

				case MIDI_PitchBend:
				{
					int val = (byte2 << 7) + byte1 - 8192;
					float normalized = val / 8192.0f;
					//		newMessage.Format((L"Bender (%.3d) %.3f"), val, normalized );
					oss << L"Bender  (" << val << ") " << normalized;
					newMessage = oss.str();
				}
				break;

				case 0:
					newMessage = (L"Zeros bytes!");
					break;

				case MIDI_SystemMessage:
				{
					switch (midiMessage[0])
					{
					case MIDI_SystemMessage: // SYSEX
					{
						std::wostringstream oss;
						oss.precision(2);

						// Tuning?
						if ((midiMessage[1] == MIDI_Universal_Realtime || midiMessage[1] == MIDI_Universal_NonRealtime) &&
							midiMessage[3] == MIDI_Sub_Id_Tuning_Standard)
						{
							int tuningCount = 0;
							int tuningDataPosition = 0;
							bool entireBank = false;
							bool retunePlayingNotes = true;

							switch (midiMessage[4])
							{
							case 1:
								/*
								[BULK TUNING DUMP]
								A bulk tuning dump comprises frequency data in the 3-byte format outlined in section 1, for all 128 MIDI key numbers, in order from note 0 (earliest sent) to note 127 (latest sent), enclosed by a system exclusive header and tail. This message is sent by the receiving instrument in response to a tuning dump request.

								F0 7E <device ID> 08 01 tt <tuning name> [xx yy zz] ... chksum F7

								F0 7E  Universal Non-Real Time SysEx header
								<device ID>  ID of responding device
								08  sub-ID#1 (MIDI Tuning)
								01  sub-ID#2 (bulk dump reply)
								tt  tuning program number (0 – 127)
								<tuning name>  16 ASCII characters
								[xx yy zz]  frequency data for one note (repeated 128 times)
								chksum  cchecksum (XOR of 7E <device ID> nn tt <388 bytes>)
								F7  EOX

								*/
							{
								tuningCount = 128;
								entireBank = true;
								tuningDataPosition = 22;
							}
							break;

							case 2: // SINGLE NOTE TUNING CHANGE (REAL-TIME)
							{
								tuningCount = midiMessage[6];
								tuningDataPosition = 7;
							}
							break;

							case 3: // BULK TUNING DUMP REQUEST (BANK)
								break;

							case 4: // KEY-BASED TUNING DUMP
							{
								tuningCount = 128;
								entireBank = true;
								tuningDataPosition = 23;
							}
							break;

							case 7: // SINGLE NOTE TUNING CHANGE (REAL/NON REAL-TIME) (BANK)
							{
								/*
								This is identical to the current SINGLE NOTE TUNING CHANGE (REAL-TIME)
								except for the addition of the bank select byte (bb) and the change to
								a NON REAL -TIME header. This message allows the sender to specify a new
								tuning change that will NOT update the currently sounding notes.
								*/
								tuningCount = midiMessage[7];
								tuningDataPosition = 8;
								retunePlayingNotes = false;
							}
							break;
							}

							if (tuningCount > 1)
							{
								oss << L"TUNE (BULK)";
							}
							else
							{
								const unsigned char* tuneEntry = &(midiMessage[tuningDataPosition]);
								int midiKeyNumber = *tuneEntry++;
								int tune = (tuneEntry[0] << 14) + (tuneEntry[1] << 7) + tuneEntry[2];
								float semitone = tune / (float)0x4000;

								oss << L"TUNE: k" << midiKeyNumber << " " << semitone;
							}
						}
						else
						{
							// MIDI HD-PROTOCOL. (hd protocol)
							if (GmpiMidiHdProtocol::isWrappedHdProtocol(midiMessage, size))
							{
								int channelGroup;
								int keyNumber;
								int val_12b;
								int val_20b;
								int status;
								int midiChannel;

								GmpiMidiHdProtocol::DecodeHdMessage(midiMessage, size, status, midiChannel, channelGroup, keyNumber, val_12b, val_20b);

								switch (status)
								{
								case GmpiMidi::MIDI_NoteOn:
								{
									/*
									int velocity_12b = val_12b;
									int directPitch_20b = val_20b;

									const float oneTwelf = 1.0f / 12.0f;
									float pitch = voiceState->GetKeyTune(keyNumber) * oneTwelf - 0.75f;

									const float recip = 1.0f / (float)0xFFF;
									float velocityN = (float)velocity_12b * recip;
									*/

									oss << L"HDNoteOn: k" << keyNumber;
								}
								break;

								case GmpiMidi::MIDI_NoteOff:
								{
									/*
									int velocity_12b = val_12b;
									const float recip = 1.0f / (float)0xFFF;
									float velocityN = (float)velocity_12b * recip;
									*/

									oss << L"HDNoteOff: k" << keyNumber;
								}
								break;

								case GmpiMidi::MIDI_ControlChange:
								{
									int controllerNumber_12b = val_12b;
									int controllerValue_20b = val_20b;

									constexpr float recip = 1.0f / (float)0xFFFFF;
									float normalised = (float)controllerValue_20b * recip;

									if (keyNumber == 0xFF) // Monophonic CCs
									{
										oss << L"HDCC:" << controllerNumber_12b << " v" << normalised;
									}
									else // Polyphonic CCs (Note Expression).
									{
										oss << L"HDCC" << controllerNumber_12b << L" k" << keyNumber << " v" << normalised;
									}
								}
								break;
								}
							}
							else
							{
								// REgualr SYSEX
								int c = min(size, 10);
								oss << L"SYSEX";

								for (int i = 0; i < c; i++)
								{
									oss << L" " << std::hex << (int)midiMessage[i];
								}

								if (c < size)
									oss << (L"...");
							}
						}

						newMessage = oss.str();
					}
					break;

					case 0xF1:
						oss << L"F1 " << std::hex << midiMessage[1] << " (MTC QF)";
						newMessage = oss.str();
						break;

					case 0xF2:
						//newMessage.Format((L"F2 %X%X (SPP)"), midiMessage[1], midiMessage[2]);
						oss << L"F2 " << std::hex << midiMessage[1] << std::hex << midiMessage[2] << " (SPP)";
						newMessage = oss.str();
						break;

					case 0xF3:
						//newMessage.Format((L"F3 %X (SS)"), midiMessage[1]);
						oss << L"F3 " << std::hex << midiMessage[1] << " (SS)";
						newMessage = oss.str();
						break;

					case 0xF6:
						newMessage = L"F6 - Tune Request";
						break;

					case 0xF8:
						newMessage = L"F8 - CLOCK";
						break;

					case 0xFA:
						newMessage = L"FA - CLOCK - Start";
						break;

					case 0xFB:
						newMessage = L"FB - CLOCK - Continue";
						break;

					case 0xFC:
						newMessage = L"FC - CLOCK - Stop";
						break;

					case 0xFE:
						newMessage = L"FE - Active Sensing";
						break;

					case 0xFF:
						newMessage = L"FF - System Reset";
						break;

					default:
						//newMessage.Format((L"SYSTEM (%x)"), midiMessage[0]);
						oss << L"SYSTEM (" << std::hex << midiMessage[1] << ")";
						newMessage = oss.str();
					};
				}
				break;

				default:
					//newMessage.Format((L"Byte = %02x %02x %02x"), stat, byte1, byte2 );
					oss << L"Byte = " << std::hex << stat << " " << std::hex << byte1 << " " << std::hex << byte2;
					newMessage = oss.str();
				}

				if (!is_system_msg)
				{
					//std::wstring chan_st;
					//chan_st.Format((L"C%2d "),chan+1);
					std::wostringstream oss2;
					oss2 << L"C" << (chan + 1) << " ";
					// newMessage = oss.str();
					newMessage = oss2.str() + newMessage;
				}

				// add time
				int64_t seconds = samplesPassed / (int)this->getSampleRate();
				int64_t minutes = seconds / 60;
				seconds %= 60;
				int64_t samples = samplesPassed % (int64_t)getSampleRate();
				std::wstring time_st;
				//		time_st.Format((L"%01d:%02d+%05d "), minutes, seconds, samples);
				newMessage = time_st + newMessage;
				const size_t lineLength = 40;

				if (newMessage.size() > lineLength)
				{
					//					newMessage = Left(newMessage, lineLength) + std::wstring(L"\n                            ") + Right(newMessage, newMessage.size() - lineLength);
					newMessage = newMessage.substr(lineLength) + std::wstring(L"\n                            ") + newMessage.substr(lineLength);
				}

				//				newMessage += L"\n";
				//		_RPTW1(_CRT_WARN, L"midimon: %s\n", newMessage );
				//				Print(newMessage);
			}
		}
		lines.push_back(newMessage);
		if (lines.size() > 14)
			lines.pop_front();

		std::wstring printout;
		for (auto& s : lines)
			printout += s + +L"\n";

		//		msg = newMessage /*+ timeText */+ L"\n" + msg + L"\n";

		pinDispOut = printout;
	}
};
