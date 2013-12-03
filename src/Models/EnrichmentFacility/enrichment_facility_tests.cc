// enrichment_facility_tests.cc
#include <gtest/gtest.h>

#include <sstream>

#include "commodity.h"
#include "facility_model_tests.h"
#include "mat_query.h"
#include "model_tests.h"
#include "resource_helpers.h"
#include "xml_query_engine.h"

#include "enrichment_facility_tests.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::SetUp() {
  cyclus::Context* ctx = tc_.get();
  src_facility = new EnrichmentFacility(ctx);

  InitParameters();
  SetUpSourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::TearDown() {
  delete src_facility;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::InitParameters() {
  cyclus::Context* ctx = tc_.get();

  in_commod = "incommod";
  out_commod = "outcommod";

  in_recipe = "recipe";
  feed_assay = 0.0072;

  cyclus::CompMap v;
  v[92235] = feed_assay;
  v[92238] = 1 - feed_assay;
  recipe = cyclus::Composition::CreateFromAtom(v);
  ctx->AddRecipe(in_recipe, recipe);

  tails_assay = 0.002;
  swu_capacity = 100;
  inv_size = 5;
  commodity_price = 0;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::SetUpSourceFacility() {
  src_facility->in_recipe(in_recipe);
  src_facility->in_commodity(in_commod);
  src_facility->out_commodity(out_commod);
  src_facility->tails_assay(tails_assay);
  src_facility->feed_assay(feed_assay);
  src_facility->commodity_price(commodity_price);
  src_facility->SetMaxInventorySize(inv_size);
  src_facility->swu_capacity(swu_capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacilityTest::GetMat(double qty) {
  return cyclus::Material::CreateUntracked(qty,
                                           tc_.get()->GetRecipe(in_recipe));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacilityTest::GetReqMat(double qty,
                                                        double enr) {
  cyclus::CompMap v;
  v[92235] = enr;
  v[92238] = 1 - enr;
  return cyclus::Material::CreateUntracked(
      qty, cyclus::Composition::CreateFromAtom(v));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::DoAddMat(cyclus::Material::Ptr mat) {
  src_facility->AddMat_(mat);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacilityTest::DoRequest() {
  return src_facility->Request_();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr
EnrichmentFacilityTest::DoOffer(cyclus::Material::Ptr mat) {
  return src_facility->Offer_(mat);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, InitialState) {
  EXPECT_EQ(in_recipe, src_facility->in_recipe());
  EXPECT_EQ(in_commod, src_facility->in_commodity());
  EXPECT_EQ(out_commod, src_facility->out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay, src_facility->tails_assay());
  EXPECT_DOUBLE_EQ(feed_assay, src_facility->feed_assay());
  EXPECT_DOUBLE_EQ(inv_size, src_facility->MaxInventorySize());
  EXPECT_DOUBLE_EQ(commodity_price, src_facility->commodity_price());
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  EXPECT_DOUBLE_EQ(swu_capacity, src_facility->swu_capacity());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, XMLInit) {
  std::stringstream ss;
  ss << "<start>"
     << "  <input>"
     << "    <incommodity>" << in_commod << "</incommodity>"
     << "    <inrecipe>" << in_recipe << "</inrecipe>"
     << "    <inventorysize>" << inv_size << "</inventorysize>"
     << "  </input>"
     << "  <output>"
     << "    <outcommodity>" << out_commod << "</outcommodity>"
     << "    <tails_assay>" << tails_assay << "</tails_assay>"
     << "    <swu_capacity>" << swu_capacity << "</swu_capacity>"
     << "  </output>"
     << "</start>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::XMLQueryEngine engine(p);
  cycamore::EnrichmentFacility fac(tc_.get());

  EXPECT_NO_THROW(fac.InitModuleMembers(&engine););
  EXPECT_EQ(in_recipe, fac.in_recipe());
  EXPECT_EQ(in_commod, fac.in_commodity());
  EXPECT_EQ(out_commod, fac.out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay, fac.tails_assay());
  EXPECT_DOUBLE_EQ(feed_assay, fac.feed_assay());
  EXPECT_DOUBLE_EQ(inv_size, fac.MaxInventorySize());
  EXPECT_DOUBLE_EQ(commodity_price, fac.commodity_price());
  EXPECT_DOUBLE_EQ(0.0, fac.InventorySize());
  EXPECT_DOUBLE_EQ(swu_capacity, fac.swu_capacity());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Clone) {
  cyclus::Context* ctx = tc_.get();

  cycamore::EnrichmentFacility* cloned_fac =
    dynamic_cast<cycamore::EnrichmentFacility*>(src_facility->Clone());

  EXPECT_EQ(in_recipe, cloned_fac->in_recipe());
  EXPECT_EQ(in_commod, cloned_fac->in_commodity());
  EXPECT_EQ(out_commod, cloned_fac->out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay, cloned_fac->tails_assay());
  EXPECT_DOUBLE_EQ(feed_assay, cloned_fac->feed_assay());
  EXPECT_DOUBLE_EQ(inv_size, cloned_fac->MaxInventorySize());
  EXPECT_DOUBLE_EQ(commodity_price, cloned_fac->commodity_price());
  EXPECT_DOUBLE_EQ(0.0, cloned_fac->InventorySize());
  EXPECT_DOUBLE_EQ(swu_capacity, cloned_fac->swu_capacity());
  
  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, AddMat) {
  EXPECT_THROW(DoAddMat(test_helpers::get_mat()), cyclus::StateError);
  EXPECT_THROW(DoAddMat(GetMat(inv_size + 1)), cyclus::ValueError);
  EXPECT_NO_THROW(DoAddMat(GetMat(inv_size)));
  EXPECT_THROW(DoAddMat(GetMat(1)), cyclus::ValueError);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(EnrichmentFacilityTest, Request) {
  double req = inv_size;
  double add = 0;
  cyclus::Material::Ptr mat = DoRequest();
  EXPECT_DOUBLE_EQ(mat->quantity(), req);
  EXPECT_EQ(mat->comp(), tc_.get()->GetRecipe(in_recipe));

  add = 2 * inv_size / 3;
  req -= add;
  DoAddMat(GetMat(add));
  mat = DoRequest();
  EXPECT_DOUBLE_EQ(mat->quantity(), req);
  EXPECT_EQ(mat->comp(), tc_.get()->GetRecipe(in_recipe));
  
  add = inv_size / 3;
  req = 0;
  DoAddMat(GetMat(add));
  mat = DoRequest();
  EXPECT_DOUBLE_EQ(mat->quantity(), req);
  EXPECT_EQ(mat->comp(), tc_.get()->GetRecipe(in_recipe));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(EnrichmentFacilityTest, Offer) {
  using cyclus::CompMap;
  using cyclus::Composition;
  using cyclus::Material;
  using cyclus::MatQuery;
  
  double qty = 4.5;
  double u234 = 1.0;
  double u235 = 1.0;
  double u238 = 2.0;
  cyclus::CompMap v;
  v[92234] = u234;
  v[92235] = u235;
  v[92238] = u238;
  Material::Ptr mat =
      DoOffer(Material::CreateUntracked(qty, Composition::CreateFromAtom(v)));

  MatQuery q(mat);

  EXPECT_DOUBLE_EQ(q.atom_frac(92234), 0.0);
  EXPECT_DOUBLE_EQ(q.atom_frac(92235), u235 / (u235 + u238));
  EXPECT_DOUBLE_EQ(q.atom_frac(92238), u238 / (u235 + u238));
  EXPECT_DOUBLE_EQ(mat->quantity(), qty);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(EnrichmentFacilityTest, ValidReq) {
  using cyclus::CompMap;
  using cyclus::Composition;
  using cyclus::Material;

  double qty = 4.5; // some magic number
  
  cyclus::CompMap v1;
  v1[92235] = 1;
  Material::Ptr mat = Material::CreateUntracked(qty,
                                                Composition::CreateFromAtom(v1));
  EXPECT_TRUE(!src_facility->ValidReq(mat)); // u238 = 0
  
  cyclus::CompMap v2;
  v2[92235] = tails_assay;
  v2[92238] = 1 - tails_assay;
  mat = Material::CreateUntracked(qty, Composition::CreateFromAtom(v2));
  EXPECT_TRUE(!src_facility->ValidReq(mat)); // u235 / (u235 + u238) <= tails_assay

  cyclus::CompMap v3;
  v3[92235] = 1;
  v3[92238] = 1;
  mat = Material::CreateUntracked(qty, Composition::CreateFromAtom(v3));
  EXPECT_TRUE(src_facility->ValidReq(mat)); // valid
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, EmptyRequests) {
  using cyclus::Material;
  using cyclus::RequestPortfolio;

  src_facility->SetMaxInventorySize(src_facility->InventorySize());
  std::set<RequestPortfolio<Material>::Ptr> ports =
      src_facility->AddMatlRequests();
  ports = src_facility->AddMatlRequests();
  EXPECT_TRUE(ports.empty());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(EnrichmentFacilityTest, AddRequests) {
  using cyclus::Request;
  using cyclus::RequestPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::Material;

  // a request is made for the current available inventory amount
  
  std::set<RequestPortfolio<Material>::Ptr> ports =
      src_facility->AddMatlRequests();

  ASSERT_EQ(ports.size(), 1);
  ASSERT_EQ(ports.begin()->get()->qty(), inv_size);

  const std::vector<Request<Material>::Ptr>& requests =
      ports.begin()->get()->requests();
  ASSERT_EQ(requests.size(), 1);

  Request<Material>::Ptr req = *requests.begin();
  EXPECT_EQ(req->requester(), src_facility);
  EXPECT_EQ(req->commodity(), in_commod);

  const std::set< CapacityConstraint<Material> >& constraints =
      ports.begin()->get()->constraints();
  CapacityConstraint<Material> c(inv_size);
  EXPECT_EQ(constraints.size(), 1);
  EXPECT_EQ(*constraints.begin(), c);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Accept) {
  using cyclus::Bid;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::Trade;
  using test_helpers::trader;

  // an enrichment facility gets two trades, each for 1/3 of its inv size
  // note that comp != recipe is covered by AddMat tests
  // note that qty >= inv capacity is covered by ResourceBuff tests
  
  double qty = inv_size / 3;
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> > responses;

  Request<Material>::Ptr req1(
      new Request<Material>(DoRequest(), src_facility, in_commod));
  Bid<Material>::Ptr bid1(new Bid<Material>(req1, GetMat(qty), &trader));

  Request<Material>::Ptr req2(
      new Request<Material>(DoRequest(), src_facility, in_commod));
  Bid<Material>::Ptr bid2(new Bid<Material>(req2, GetMat(qty), &trader));

  Trade<Material> trade1(req1, bid1, qty);
  responses.push_back(std::make_pair(trade1, GetMat(qty)));
  Trade<Material> trade2(req2, bid2, qty);
  responses.push_back(std::make_pair(trade2, GetMat(qty)));

  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  src_facility->AcceptMatlTrades(responses);  
  EXPECT_DOUBLE_EQ(qty * 2, src_facility->InventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, AddBids) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::ExchangeContext;
  using cyclus::Material;

  // an enrichment facility bids on nreqs requests
  // note that bid response is covered by Bid tests  
  // note that validity of requests is covered by ValidReq tests 
  int nreqs = 5;
  int nvalid = 4;
  
  // set up inventory
  double current_size = inv_size / 2; // test something other than max size
  DoAddMat(GetMat(current_size));
  
  boost::shared_ptr< cyclus::ExchangeContext<Material> >
      ec = GetContext(nreqs, nvalid);
  
  std::set<BidPortfolio<Material>::Ptr> ports =
      src_facility->AddMatlBids(ec.get());

  ASSERT_TRUE(ports.size() > 0);
  EXPECT_EQ(ports.size(), 1);

  BidPortfolio<Material>::Ptr port = *ports.begin();
  EXPECT_EQ(port->bidder(), src_facility);
  EXPECT_EQ(port->bids().size(), nvalid);
  
  const std::set< CapacityConstraint<Material> >& constrs = port->constraints();
  Converter<Material>::Ptr sc(new SWUConverter(feed_assay, tails_assay));
  Converter<Material>::Ptr nc(new NatUConverter(feed_assay, tails_assay));
  CapacityConstraint<Material> swu(swu_capacity, sc);
  CapacityConstraint<Material> natu(current_size, nc);
  EXPECT_EQ(constrs.size(), 2);
  EXPECT_TRUE(*constrs.begin() == swu || *(++constrs.begin()) == swu);
  EXPECT_TRUE(*constrs.begin() == natu || *(++constrs.begin()) == natu);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
EnrichmentFacilityTest::GetContext(int nreqs, int nvalid) {
  using cyclus::ExchangeContext;
  using cyclus::Material;
  using cyclus::Request;
  using test_helpers::trader;
  using test_helpers::get_mat;
  
  boost::shared_ptr< ExchangeContext<Material> >
      ec(new ExchangeContext<Material>());
  for (int i = 0; i < nvalid; i++) {
    ec->AddRequest(Request<Material>::Ptr(
        new Request<Material>(GetReqMat(1.0, 0.05), &trader, out_commod)));
  }  
  for (int i = 0; i < nreqs - nvalid; i++) {
    ec->AddRequest(Request<Material>::Ptr(
        // get_mat returns a material of only u235, which is not valid
        new Request<Material>(get_mat(), &trader, out_commod)));
  }
  return ec;
}

// //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TEST_F(SourceFacilityTest, Response) {
//   using cyclus::Bid;
//   using cyclus::Material;
//   using cyclus::Request;
//   using cyclus::Trade;
//   using test_helpers::trader;
//   using test_helpers::get_mat;

//   std::vector< cyclus::Trade<cyclus::Material> > trades;
//   std::vector<std::pair<cyclus::Trade<cyclus::Material>,
//                         cyclus::Material::Ptr> > responses;

//   // Null response
//   EXPECT_NO_THROW(src_facility->PopulateMatlTradeResponses(trades, responses));
//   EXPECT_EQ(responses.size(), 0);
  
//   double qty = capacity / 3;
//   Request<Material>::Ptr request(
//       new Request<Material>(get_mat(), &trader, commod));
//   Bid<Material>::Ptr bid(new Bid<Material>(request, get_mat(), src_facility));

//   Trade<Material> trade(request, bid, qty);
//   trades.push_back(trade);

//   // 1 trade, SWU < SWU cap, NatU < NatU cap
//   ASSERT_EQ(src_facility->current_capacity(), capacity);
//   src_facility->PopulateMatlTradeResponses(trades, responses);
//   EXPECT_EQ(responses.size(), 1);
//   EXPECT_EQ(responses[0].second->quantity(), qty);
//   EXPECT_EQ(responses[0].second->comp(), recipe);

//   // 1 trade, SWU > SWU Cap, NatU < NatU cap
//   ASSERT_DOUBLE_EQ(src_facility->current_capacity(), capacity - qty);
//   ASSERT_GT(src_facility->current_capacity() - 2 * qty, -1 * cyclus::eps());
//   trades.push_back(trade);
//   responses.clear();
//   EXPECT_NO_THROW(src_facility->PopulateMatlTradeResponses(trades, responses));
//   EXPECT_EQ(responses.size(), 2);
//   ASSERT_TRUE(cyclus::AlmostEq(src_facility->current_capacity(), 0));

//   // 1 trade, SWU < SWU Cap, NatU > NatU cap
//   // 1 trade, SWU = SWU Cap, NatU < NatU cap
//   // 1 trade, SWU < SWU Cap, NatU = NatU cap
  
//   // // too much qty, capn!
//   // EXPECT_THROW(src_facility->PopulateMatlTradeResponses(trades, responses),
//   //              cyclus::StateError);
  
//   // // reset!
//   // src_facility->HandleTick(1);
//   // ASSERT_DOUBLE_EQ(src_facility->current_capacity(), capacity);
// }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* EnrichmentFacilityModelConstructor(cyclus::Context* ctx) {
  using cycamore::EnrichmentFacility;
  return dynamic_cast<cyclus::Model*>(new EnrichmentFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* EnrichmentFacilityConstructor(cyclus::Context* ctx) {
  using cycamore::EnrichmentFacility;
  return dynamic_cast<cyclus::FacilityModel*>(new EnrichmentFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(EnrichmentFac, FacilityModelTests,
                        Values(&EnrichmentFacilityConstructor));
INSTANTIATE_TEST_CASE_P(EnrichmentFac, ModelTests,
                        Values(&EnrichmentFacilityModelConstructor));

} // namespace cycamore
