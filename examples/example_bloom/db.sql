create table test (
id int pk,
val int bloom
);

create procedure add(@id int in, @val int in)
begin
insert table test values (@id, @val);
end;

create procedure get(@val int in, @oid int out, @oval int out)
begin
select id set @oid, val set @oval from test where val=@val or not val<=@val;
end;
