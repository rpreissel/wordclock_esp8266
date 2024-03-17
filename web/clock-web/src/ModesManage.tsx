import { Button, Form } from "react-bootstrap";

import { Configs } from "./types";
import { useState } from "react";

type ModesManageProps = {
    modes: { index: number, type: string, name: string }[];
    configs: Configs;
    onSave: (modes: { index: number, type: string, name: string }[]) => void;
};

const ModesManage = ({ modes, configs, onSave }: ModesManageProps) => {
    const [{ modes: newModes, changed }, setState] = useState({ modes, changed: false });
    return <Form onSubmit={e => {
        e.preventDefault();
        onSave(newModes);
        setState({ modes: newModes, changed: false });
    }} onReset={e => {
        e.preventDefault();
        setState({ modes, changed: false });
    }}>
        <Form.Group className="mb-3" controlId="formModes">
            <div className="d-flex">
                <div className="d-inline-block p-1">
                    <Form.Label>Name / Type</Form.Label>
                </div>
            </div>
            {newModes.map((mode, i) => {
                return (<div key={i} className="d-flex">
                    <div className="d-inline-block p-1">
                        <Form.Control type="text" value={mode.name} onChange={e => {
                            const newMode = { ...mode, name: e.currentTarget.value }
                            newModes[i] = newMode;
                            setState({ modes: newModes, changed: true });
                        }} />
                    </div>
                    <div className="d-inline-block flex-fill p-1">
                        <Form.Select value={mode.type} onChange={e => {
                            const newName = mode.name && mode.name != mode.type ? mode.name : e.currentTarget.value;
                            const newMode = { ...mode, type: e.currentTarget.value, name: newName }
                            newModes[i] = newMode;
                            setState({ modes: newModes, changed: true });
                        }}>
                            {
                                configs.types
                                    .map((t, ti) => {
                                        return <option key={ti} value={t}>{t}</option>;
                                    })

                            }
                        </Form.Select>
                    </div>
                </div>);
            })}
        </Form.Group>
        <Button variant="primary" type="submit" className="me-2" disabled={!changed}>
            Submit
        </Button>
        <Button variant="secondary" type="reset" disabled={!changed}>
            Reset
        </Button>

    </Form>

};

export default ModesManage;