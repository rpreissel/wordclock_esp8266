import { Accordion, AccordionContext, Container } from "react-bootstrap";
import { PropsWithChildren, useContext } from "react";
import { modeFromIndex, modeName } from "./types.ts";

import LiveView from "./LiveView.tsx";
import ModeEdit from "./edit/ModeEdit.tsx";
import ModeThumbnail from "./ModeThumbnail.tsx";
import ModesManage from "./ModesManage.tsx";
import { modeIndexToColorHex } from "./colors.ts";
import { useModel } from "./model.ts";

type AccordionPartProps = {
  eventKey: string;
  header: string
};
const AccordionPart = ({ eventKey, header, children }: PropsWithChildren<AccordionPartProps>) => {
  const { activeEventKey } = useContext(AccordionContext);
  return <Accordion.Item eventKey={eventKey}>
    <Accordion.Header>{header}</Accordion.Header>
    <Accordion.Body>
      {activeEventKey && (activeEventKey as string[]).includes(eventKey) ? children : <></>}
    </Accordion.Body>
  </Accordion.Item>
};
function App() {
  const [model] = useModel();
  console.log("render")
  if (!model) {
    return <div>Loading</div>;
  }

  return (
    <Container fluid >
      <Accordion defaultActiveKey={["Modes"]} alwaysOpen>
        <AccordionPart eventKey="Modes" header="Modus auswählen">
          <div className="d-flex justify-content-center">
            <div style={{ width: "fit-content", margin: "auto" }}>

              {model.modes.map((mode) => {
                const colorHex = modeIndexToColorHex(mode.index,model.modes, model.colors);
                return (<ModeThumbnail key={mode.index} mode={mode} color={colorHex} active={model.current == mode.index}
                  onClick={index => model.current = index} />);
                  
                })}
                <ModeThumbnail key={-1} mode={modeFromIndex(-1,model.modes)} color="lightgray" active={model.current == -1}
                  onClick={index => model.current = index} />
            </div>
          </div>
        </AccordionPart>
        {model.current >= 0 && (<>
          <AccordionPart eventKey="Configure" header={`Modus '${modeName(model.currentMode)}' konfigurieren`}>
            <div className="d-flex justify-content-center">
              <ModeEdit key={model.currentMode.type + model.current} mode={model.currentMode} configs={model} modes={model.modes} onSave={(mode) => model.changeMode(mode)} />
            </div>
          </AccordionPart>
        </>)}
        <AccordionPart eventKey="View" header="Live-Bild und Einstellungen">
          <div className="d-flex justify-content-center">
            <LiveView modes={model.modes} configs={model} fixedTime={model.fixedTime} 
            onResetWifi={model.resetWifi}
            onResetData={model.resetData}
            onChange={fixedTime => model.fixedTime = fixedTime} />
          </div>
        </AccordionPart>
        <AccordionPart eventKey="Organize" header="Modi konfigurieren">
          <div className="d-flex justify-content-center">
            <ModesManage key={model.version} modes={model.modes.map(m => ({ index: m.index, type: m.type, name: modeName(m) }))}
              configs={model} onSave={modes => model.changeModes(modes)} />
          </div>
        </AccordionPart>
      </Accordion>
    </Container>
  );
}

export default App;
