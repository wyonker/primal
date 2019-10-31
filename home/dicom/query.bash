#echo "select * from patient left join (receive, process, send) on (receive.puid=patient.puid and process.puid=patient.puid and send.puid=patient.puid);"|mysql primal;

echo "select a.puid, a.pname, a.pid, a.paccn, a.pdob, a.sdatetime, r.tstartrec, r.tendrec, r.timages, r.terror, p.tstartproc, p.tendproc, p.terror, s.tdest, s.tstartsend, s.tendsend, s.timages, s.terror from patient as a left join receive as r on a.puid = r.puid left join process as p on a.puid = p.puid left join send as s on a.puid = s.puid;"|$DBCONN;
