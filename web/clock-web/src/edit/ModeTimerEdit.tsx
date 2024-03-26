import { BaseMode, Configs, Mode, TimerMode } from "../types";
import { Button, Form } from "react-bootstrap";

type ModeTimerEditProps = {
  mode: TimerMode;
  modes: Mode[];
  configs: Configs;
  onChange: (mode: TimerMode) => void;
};
export function ModeTimerEdit({ mode, modes, onChange }: ModeTimerEditProps) {
  return <>
    <Form.Group className="mb-1" controlId="formMainMode">
      <Form.Label>Standart-Modus</Form.Label>
      <Form.Select value={mode.mainMode} onChange={e => {
        onChange({ ...mode, mainMode:+e.currentTarget.value });
      }}>
        {modes
          .filter((m) => m.type !== "EMPTY")
          .filter(m => m.index != mode.index)
          .map(m => m as BaseMode)
          .map((m, mi) => {
            return <option key={mi} value={m.index}>{m.name}</option>;
          })}
        <option key={-1} value={-1}>Aus</option>;
      </Form.Select>
    </Form.Group>
    <Form.Group className="mb-3" controlId="formTimer">
      <div className="d-flex flex-wrap">
        <div className="d-inline-block w-50 p-1">
          <Form.Label>Startzeit</Form.Label>
        </div>
        <div className="d-inline-block w-50 p-1">
          <Form.Label>Endezeit</Form.Label>
        </div>
      </div>
      {mode.timers.map((timer, i) => {
        return (<div key={i} className="d-flex flex-wrap ">
          <div className="d-inline-block w-50 p-1">
            <Form.Control type="time" value={timer.startHour.toString().padStart(2, '0') + ":" + timer.startMinute.toString().padStart(2, '0')} onChange={e => {
              const [newHour, newMinute] = e.currentTarget.value.split(":");
              const newTimer = { ...timer, startHour: +newHour, startMinute: +newMinute };
              const newTimers = [...mode.timers];
              newTimers[i] = newTimer;
              onChange({ ...mode, timers: newTimers });
            }} />
          </div>
          <div className="d-inline-block w-50 p-1">
            <Form.Control type="time" value={timer.endHour.toString().padStart(2, '0') + ":" + timer.endMinute.toString().padStart(2, '0')} onChange={e => {
              const [newHour, newMinute] = e.currentTarget.value.split(":");
              const newTimer = { ...timer, endHour: +newHour, endMinute: +newMinute };
              const newTimers = [...mode.timers];
              newTimers[i] = newTimer;
              onChange({ ...mode, timers: newTimers });
            }} />
          </div>
          <div className="d-inline-block flex-fill p-1">
            <Form.Select value={timer.mode} onChange={e => {
              const newTimer = { ...timer, mode: +e.currentTarget.value };
              const newTimers = [...mode.timers];
              newTimers[i] = newTimer;
              onChange({ ...mode, timers: newTimers });
            }}>
              {modes
                .filter((m) => m.type !== "OFF")
                .filter((m) => m.type !== "EMPTY")
                .filter(m => m.index != mode.index)
                .map(m => m as BaseMode)
                .map((m, mi) => {
                  return <option key={mi} value={m.index}>{m.name}</option>;
                })}
              <option key={-1} value={-1}>Aus</option>;
            </Form.Select>
          </div>
          <div className="d-inline-block w-25 p-1">
            <Button className="float-end" type="button" variant="danger" disabled={mode.timers.length < 2} onClick={() => {
              const newTimers = [...mode.timers];
              newTimers.splice(i, 1);
              onChange({ ...mode, timers: newTimers });
            }}>
              Löschen
            </Button>
          </div>
        </div>);
      })}
      <div className="d-flex justify-content-end">
        <div className="d-inline-block w-25 p-1">
          <Button type="button" variant="info" className="float-end" onClick={() => {
            const newTimers = [...mode.timers];
            newTimers.push({ mode: -1, startHour: 0, startMinute: 0, endHour: 1, endMinute: 0 });
            onChange({ ...mode, timers: newTimers });
          }}>
            Neu
          </Button>
        </div>
      </div>
    </Form.Group>
  </>;
}
