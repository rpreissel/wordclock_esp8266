import { BaseMode, Configs, Mode, WordClockMode } from "./types";
import { Button, Form } from "react-bootstrap";

import { ColorChooser } from "./Colors";
import { useState } from "react";

type ModeEditProps = {
  mode: Mode;
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
          const times = {...mode.times}
          times[timedIndex]=e.currentTarget.value;
          console.log(times);
          onChange({...mode, times:times});
        }}>
          {go.map((t,i) => <option key={i} value={t}>{t}</option>)}
        </Form.Select>
      })}
    </Form.Group>
  </>;
}

function ModeEdit({ mode, onSave, ...props }: ModeEditProps) {
  const [state, setState] = useState(mode);
  if (state.type == "EMPTY") {
    return <></>;
  }
  let formContent = <></>;
  if (state.type == "DIGICLOCK") {
    formContent = <ModeBaseEdit mode={state} {...props} onChange={mode => setState(mode)} />;
  }
  if (state.type == "WORDCLOCK") {
    formContent = <>
      <ModeBaseEdit mode={state} {...props} onChange={mode => setState(mode)} />
      <ModeWordClockEdit mode={state} {...props} onChange={mode => setState(mode)} />
    </>;
  }
  if (state.type == "PICTURE") {
    formContent = <ModeBaseEdit mode={state} {...props} onChange={mode => setState(mode)} />;
  }
  if (state.type == "INTERVAL") {
    formContent = <ModeBaseEdit mode={state} {...props} onChange={mode => setState(mode)} hideColorBrightness />;
  }
  if (state.type == "TIMER") {
    formContent = <ModeBaseEdit mode={state} {...props} onChange={mode => setState(mode)} hideColorBrightness />;
  }
  return <Form onSubmit={e => {
    e.preventDefault();
    onSave(state);
  }} onReset={e => {
    e.preventDefault();
    setState(mode);
  }}>  {formContent}
    <Button variant="primary" type="submit" className="me-2">
      Submit
    </Button>
    <Button variant="secondary" type="reset">
      Reset
    </Button>

  </Form>

}

export default ModeEdit;
