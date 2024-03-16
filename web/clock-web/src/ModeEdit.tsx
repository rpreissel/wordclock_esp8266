import { BaseMode, Configs, Interval, IntervalMode, Mode, WordClockMode } from "./types";
import { Button, Col, Form, Row } from "react-bootstrap";

import { ColorChooser } from "./Colors";
import { useState } from "react";

type ModeEditProps = {
  mode: Mode;
  modes: Mode[];
  configs: Configs;
  onSave: (mode: Mode) => void;
};

type ModeBaseEditProps<M extends BaseMode> = {
  mode: M;
  configs: Configs;
  hideColorBrightness?: boolean;
  onChange: (mode: M) => void;
};

function ModeBaseEdit<M extends BaseMode>({ mode, configs, onChange, hideColorBrightness }: ModeBaseEditProps<M>) {
  return <>    <Form.Group className="mb-3" controlId="formName">
    <Form.Label>Name</Form.Label>
    <Form.Control type="text" value={mode.name} onChange={e => {
      onChange({ ...mode, name: e.currentTarget.value });
    }} />
  </Form.Group>
    {!hideColorBrightness &&
      (<><Form.Group className="mb-3" controlId="formBrightness">
        <Form.Label>Brightness</Form.Label>
        <Form.Range value={mode.brightness} min="1" max="255" onChange={e => {
          onChange({ ...mode, brightness: e.currentTarget.value });
        }} />
      </Form.Group>
        <Form.Group className="mb-3" controlId="formColor">
          <Form.Label>Color</Form.Label>
          <ColorChooser value={mode.color} colors={configs.colors} onChange={c =>
            onChange({ ...mode, color: c })
          } />
        </Form.Group></>)
    }
  </>;
}
type ModeWordClockEditProps = {
  mode: WordClockMode;
  configs: Configs;
  onChange: (mode: WordClockMode) => void;
};
function ModeWordClockEdit({ mode, configs, onChange }: ModeWordClockEditProps) {
  return <>
    <Form.Group className="mb-3" controlId="formDefs">
      <Form.Label>Times</Form.Label>
      {Object.keys(configs.times).map((timedIndex, i) => {
        const go = configs.times[timedIndex];
        return <Form.Select key={i} value={mode.times[timedIndex]} onChange={e => {
          const times = { ...mode.times }
          times[timedIndex] = e.currentTarget.value;
          console.log(times);
          onChange({ ...mode, times: times });
        }}>
          {go.map((t, i) => <option key={i} value={t}>{t}</option>)}
        </Form.Select>
      })}
    </Form.Group>
  </>;
}

type ModeIntervalEditProps = {
  mode: IntervalMode;
  modes: Mode[];
  configs: Configs;
  onChange: (mode: IntervalMode) => void;
};
function ModeIntervalEdit({ mode, modes, configs, onChange }: ModeIntervalEditProps) {
  return <>
    <Form.Group className="mb-3" controlId="formInterval">
      <Row>
        <Col xs="3">
          <Form.Label>Seconds</Form.Label>
        </Col>
        <Col xs="7">
          <Form.Label>Mode</Form.Label>
        </Col>
        <Col xs="2">
          <Form.Label>Action</Form.Label>
        </Col>
      </Row>
      {mode.intervals.map((interval, i) => {
        return (<Row key={i} className="mb-1">
          <Col xs="3">
            <Form.Control type="number" value={interval.seconds} onChange={e => {
              const newinterval = { ...interval, seconds: +e.currentTarget.value }
              const newIntervals = [...mode.intervals];
              newIntervals[i] = newinterval;
              onChange({ ...mode, intervals: newIntervals });
            }} />
          </Col>
          <Col xs="7">
            <Form.Select value={interval.mode} onChange={e => {
              const newinterval = { ...interval, mode: +e.currentTarget.value }
              const newIntervals = [...mode.intervals];
              newIntervals[i] = newinterval;
              onChange({ ...mode, intervals: newIntervals });
            }}>
              {
                modes
                  .filter((m) => m.type !== "OFF")
                  .filter((m) => m.type !== "EMPTY")
                  .filter(m => m.index != mode.index)
                  .map(m => m as BaseMode)
                  .map((m, mi) => {
                    return <option key={mi} value={m.index}>{m.name}</option>;
                  })

              }
              <option key={-1} value={-1}>Off</option>;
            </Form.Select>
          </Col>
          <Col xs="2">
            <Button type="button" variant="danger" disabled={mode.intervals.length < 3} size="sm" onClick={() => {
              const newIntervals = [...mode.intervals]
              newIntervals.splice(i, 1);
              onChange({ ...mode, intervals: newIntervals });
            }}>
              Delete
            </Button>
          </Col>
        </Row>);
      })}
      <Row>
        <Col xs="3">
        </Col>
        <Col xs="7">
        </Col>
        <Col xs="2">
          <Button type="button" variant="info" size="sm"  onClick={() => {
                const newIntervals = [...mode.intervals]
                newIntervals.push({seconds:10, mode:-1});
                onChange({...mode, intervals:newIntervals});
              }}>
            Add
          </Button>
        </Col>
      </Row>
    </Form.Group>
  </>;
}

function ModeEdit({ mode, onSave, ...props }: ModeEditProps) {
  const [{ mode: newMode, changed }, setState] = useState({ mode: mode, changed: false });
  if (newMode.type == "EMPTY") {
    return <></>;
  }
  let formContent = <></>;
  if (newMode.type == "DIGICLOCK") {
    formContent = <ModeBaseEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />;
  }
  if (newMode.type == "WORDCLOCK") {
    formContent = <>
      <ModeBaseEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />
      <ModeWordClockEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />
    </>;
  }
  if (newMode.type == "PICTURE") {
    formContent = <ModeBaseEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />;
  }
  if (newMode.type == "INTERVAL") {
    formContent = <>
      <ModeBaseEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} hideColorBrightness />
      <ModeIntervalEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />
    </>
  }
  if (newMode.type == "TIMER") {
    formContent = <ModeBaseEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} hideColorBrightness />;
  }
  return <Form onSubmit={e => {
    e.preventDefault();
    onSave(newMode);
    setState({ mode: newMode, changed: false });
  }} onReset={e => {
    e.preventDefault();
    setState({ mode, changed: false });
  }}>  {formContent}
    <Button variant="primary" type="submit" className="me-2" disabled={!changed}>
      Submit
    </Button>
    <Button variant="secondary" type="reset" disabled={!changed}>
      Reset
    </Button>

  </Form>

}

export default ModeEdit;
